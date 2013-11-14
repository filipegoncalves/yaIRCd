#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include "protocol.h"
#include "parsemsg.h"
#include "msgio.h"

/** @file
	@brief IRC Messages parser implementation
	
	This file implements functions that are used to parse an IRC message.
	See RFC section 2.3 to learn about IRC messages syntax.
	
	@author Filipe Goncalves
	@date November 2013
*/

/** Searches for the next position in `str` that is not a white space. Tabs are not considered white space.
	It is assumed that the target string is null terminated. If there's no next position that is not a white space,
	a pointer to the end of the string is returned.
	@param str The target string
	@return Pointer to the first position in `str` that is not a white space.
*/
static char *skipspaces(char *str) {
	for (; *str != '\0' && *str == ' '; str++)
		; /* Intentionally left blank */
	return str;
}

/** Searches for the next position in `str` that is a white space. Tabs are not considered white space.
	It is assumed that the target string is null terminated. If there's no next position that is a white space,
	a pointer to the end of the string is returned.
	@param str The target string
	@return Pointer to the first position in `str` that is a white space.
*/
static char *skipnonspaces(char *str) {
	for (; *str != '\0' && *str != ' '; str++)
		; /* Intentionally left blank */
	return str;
}

/** Reads the parameters in an IRC message.
	@param buf Pointer to the beginning of a characters sequence with the parameters.
	@param params Array of pointers where each element will point to the beginning of a parameter. Note that the last parameter may be a sequence of characters with spaces if ':' was used.
	@return <ul>
				<li>`-1` if the maximum number of parameters allowed by the protocol (15) was exceeded, in which case the contents of `params[i]` is undefined;</li>
				<li>Otherwise, the number of parameters read and inserted in `params` is returned.</li>
			</ul>
	@warning `buf` is destructively changed. In particular, white space separators will be overwritten with `NUL` characters. `params[i]` points to a position in `buf`, thus, no memory is allocated in this function, 
			  and no copies of `buf` take place. This function only manipulates pointers.
	@warning It is assumed that `buf` is null terminated, and that no \\r\\n trailing characters exist.
	@warning `params` is assumed to contain enough space for the maximum number of parameters allowed for an IRC message (as of this writing, 15). It can be bigger than that, but this function will ignore any additional space.
*/
static int read_params(char *buf, char *params[MAX_IRC_PARAMS]) {
	int pos;
	char *next;
	
	pos = 0;
	for (buf = skipspaces(buf), next = skipnonspaces(buf); *buf != '\0' && *buf != ':'; buf = skipspaces(buf), next = skipnonspaces(buf)) {
		/* assert:
			*buf != ' ' && *buf != '\0' && *buf != ':'
			*next == ' ' || *next == '\0'
			=> assert(buf != next) [assures that this is a non-empty sequence, as required by the RFC]
		*/
		if (pos == MAX_IRC_PARAMS) {
			return -1; /* Sorry buddy, no buffer overflow hacks! */
		}
		params[pos++] = buf;
		if (*next == '\0') {
			buf = next;
		}
		else {
			buf = next+1;
			*next = '\0';
		}
	}
	if (*buf == ':' && (buf = skipspaces(buf+1)) != '\0') {
		/* We allow for spaces after ':' in the parameters list. The RFC does not; but this is harmless :) */
		if (pos < MAX_IRC_PARAMS) {
			params[pos++] = buf;
		}
		else {
			return -1;
		}
	}
	return pos;
}

/** Parses an IRC messsage and splits it up into its different components. The format for an IRC message is thoroughly described in Section 2.3.1 of the IRC specification (see doc/rfc.html).
	This function acts more like a tokenizer - note that no semantic checking takes place. It is a purely syntax based parser.
	The message buffer is manipulated and its contents will be different after the function returns. Namely, space separators can be overwritten with a `NUL` character.
	This function can be safely called by different threads, as long as each thread passes different arguments.
	@param buf Buffer containing the new message. It is assumed that the buffer is null terminated.
	@param prefix When a prefix exists, `*prefix` will point to the beginning of the prefix field (first character after `:`) in `buf`. The end of the prefix is determined by a `NUL` character.
		   If there is no prefix, `*prefix` is `NULL`.
	@param cmd `*cmd` will point to the beginning of the command field (first non space character in the message; first non space character after prefix if one exists). The end is determined by a `NUL` character.
	@param params An array of pointers to a character sequence. Each array element points to a position in `buf` denoting a parameter. Parameters are null terminated.
	@param params_filled `*params_filled` will hold the number of parameters parsed. Thus, after returning, it is valid to reference any position `i` in `params` as long as `i >= 0 && i < *params_filled`.
	@return <ul>
				<li>`-1` if an error occurred, meaning there was a syntax error. Syntax errors that can be detected include: 
					 <ol>
						<li>The case that a message only contains a prefix field, in which case the value of `*prefix` is undefined;</li>
						<li>The case that there are more parameters in a command than the maximum allowed by the RFC, in which case the contents of `params[i]` is undefined, but no buffer overflow conditions occur.</li>
					</ol>
				</li>
				<li>`0` on success</li>
	@warning `buf` is destructively changed. `*prefix`, `*cmd`, and `params[i]` all point to different positions in `buf`. No memory is allocated inside this function, only pointer manipulation takes place.
	@warning When `*params_filled > 0`, note that, according to the RFC, it is possible that `params[*params_filled-1]` points to a parameter with spaces. This is the case everytime a trailing parameter was prefixed with `:`
	@warning `params` is assumed to contain enough space for the maximum number of parameters allowed for an IRC message (as of this writing, 15). It can be bigger than that, but this function will ignore any additional space.
	@warning When `-1` is returned, it is not guaranteed that `*cmd` and `*prefix` point to a null terminated characters sequence, and the value in `*params_filled` is undefined. Thus, the caller shall always check the return value
			 of this function before using any of the information it provides.
*/
int parse_msg(char *buf, char **prefix, char **cmd, char *params[MAX_IRC_PARAMS], int *params_filled) {
	char *current;
	char *next;
	int ret;
	
	current = skipspaces(buf);
	ret = 0;
	*prefix = NULL;
	if (*current == ':') {
		next = skipnonspaces(current+1);
		if (*next == '\0' || next == current+1) {
			/* Sender said there was a prefix, but there's no prefix */
			return -1;
		}
		*next = '\0';
		*prefix = current+1;
		ret = 1;
		current = next+1;
	}
	current = skipspaces(current);
	if (*current == '\0') {
		return -1;
	}
	/* Parse command */
	if (isdigit((unsigned char) *current)) {
		if (isdigit((unsigned char) *(current+1)) && isdigit((unsigned char) *(current+2)) && (*(current+3) == '\0' || *(current+3) == ' ')) {
			*cmd = current;
			next = current+3;
		}
		else {
			return -1;
		}
	}
	else {
		for (next = current; *next != '\0' && isalpha((unsigned char) *next); next++)
			; /* Intentionally left blank */
		if (*next != '\0' && *next != ' ') {
			/* Invalid command */
			return -1;
		}
	}
	*params_filled = 0;
	/* assert: *next == ' ' || *next == '\0' */
	if (*next != '\0' && (*params_filled = read_params(next+1, params)) == -1) {
		return -1;
	}
	*next = '\0';
	*cmd = current;
	return ret;
}

/** Initializes a `struct irc_message`, typically from a client.
	@param in The structure to initialize.
*/
void initialize_irc_message(struct irc_message *in) {
	in->status = 0;
	in->index = 0;
	in->last_stop = 0;
	in->msg_begin = 0;
};

/** Copies every characters from `buf[0..length-1] to `to`. Assumes `to` has enough space, which is safe because this function is only used inside this file
	as an auxiliary function from `next_msg()`.
	@param to Pointer to the beginning of the target buffer.
	@param buf Pointer to the buffer being copied.
	@param length How many characters to copy from `buf`.
	@note `to` and `buf` may overlap in memory, and in fact that is the case. `next_msg()` uses this function to bring a message to the front of the queue.
*/
static void bring_to_top(char *to, char *buf, int length) {
	if (buf == to) {
		return;
	}
	for (; length-- > 0; *to++ = *buf++)
		; /* Intentionally left blank */
}

/** Called everytime there is new data to read from the socket. After calling this function, it is advised to use `next_msg()` to retrieve the IRC messages that can be extracted from this read,
	otherwise, the caller risks losing space in the messages buffer.
	@param client The client that transmitted new data.
	@note This function never overflows. If it is called repeatedly without calling `next_msg()`, it will eventually run out of space and throw away everything read, emptying the buffer.
	@note This function only eads what it can. This means that, for example, there may be 256 characters available to read from the socket, but if there's only space to read 28, only 28 are read.
	This is generally the case when IRC messages were fragmented and we are waiting for the rest of some message, which means our buffer is not empty.
	The function shall be called again if it is known that there is more data in the socket to parse, but only after calling `next_msg()` to free some space in the buffer.
*/
void read_data(struct irc_client *client) {
	struct irc_message *client_msg = client->last_msg;
	char *buf = client_msg->msg+client_msg->index;
	if (sizeof(client_msg->msg) <= client_msg->index) {
		/* If we get here, it means we have read a characters sequence of at least MAX_MSG_SIZE length without finding
		   the message terminators \r\n. A lame client is messing around with the server. Reset the message buffer and log
		   this malicious behavior.
		 */
		fprintf(stderr, "Parse error: message exceeds maximum allowed length. Received by %s\n", client->nick == NULL ? "<unregistered>" : client->nick);
		initialize_irc_message(client_msg);
	}
	client_msg->index += read_from(client, buf, sizeof(client_msg->msg)-client_msg->index);
}

/** Analyzes the incoming messages buffer and the information read from the socket to determine if there's any IRC message that can be retrieved from the buffer at the moment.
	@param client_msg The structure representing state information for the sockets reading performed earlier.
	@param msg If a new message is available, `*msg` will point to the beginning of a characters sequence that holds an IRC message terminated with \\r\\n.
	@return This function returns a negative constant if no new message is available. Possible values for this constant are:
			<ul>
				<li>`MSG_CONTINUE` - this means that, at the moment, it is not possible to retrieve a complete IRC message, and that the caller shall wait until there is more incoming data in the socket</li>
				<li>`MSG_FINISH_ERR` - a message that can't possibly be well formed has been detected. This often happens when more than `MAX_MSG_SIZE` characters have been read without a terminating sequence.</li>
			</ul>
			In case of success, the the length of a new IRC message is returned, and `*msg` will point to the beginning of the message. Any access in `(*msg)[0..length-1]` is valid. On success, `length` is guaranteed
			to be greater than or equal to 2, since an IRC message is terminated by \\r\\n. Thus, it is safe for the upper caller to null-terminate an IRC message by executing `(*msg)[length-2] = &lsquo;\\0&rsquo;`.
	@warning No space allocation takes place, only pointer manipulation.
	@warning Always check for the return codes for this function before using `*msg`. Its contents are undefined when the return value is not a positive integer.
*/
int next_msg(struct irc_message *client_msg, char **msg) {
	int i;
	int len;
	char *buf = client_msg->msg;
	for (i = client_msg->last_stop; client_msg->status != (STATUS_SEEN_CR | STATUS_SEEN_LF) && i < client_msg->index; i++) {
		if (buf[i] == '\r') {
			client_msg->status |= STATUS_SEEN_CR;
		}
		else if (buf[i] == '\n') {
			client_msg->status |= STATUS_SEEN_LF;
		}
	}
	/* assert: i == client_msg->index || client_msg->status == (STATUS_SEEN_CR | STATUS_SEEN_LF) */
	if (client_msg->status == (STATUS_SEEN_CR | STATUS_SEEN_LF)) {
		/* assert: (buf[i-1] == '\r' || buf[i-1] == '\n') && i-1 < client_msg->index */
		client_msg->status = 0;
		len = i - client_msg->msg_begin;
		*msg = client_msg->msg+client_msg->msg_begin;
		client_msg->last_stop = i;
		client_msg->msg_begin = i;
		if (len >= 2 && buf[i-1] == '\n' && buf[i-2] == '\r') {
			return len;
		} else {
			initialize_irc_message(client_msg);
			return MSG_FINISH_ERR;
		}
	}
	else {
		/* assert: i == client_msg->index  && client_msg->status != (STATUS_SEEN_CR | STATUS_SEEN_LF) */
		bring_to_top(client_msg->msg, client_msg->msg+client_msg->msg_begin, client_msg->index -= client_msg->msg_begin);
		client_msg->last_stop = client_msg->index;
		client_msg->msg_begin = 0;
		return MSG_CONTINUE;
	}
}

/* Debug */
/*
#include <stdio.h>
#include <string.h>

void prtbuf(char *b) {
	for (; *b != '\0'; b++) {
		if (*b == '\r') {
			putchar('\\');
			putchar('r');
		}
		else if (*b == '\n') {
			putchar('\\');
			putchar('n');
		}
		else
			putchar(*b);
	}
	putchar('\n');
}


int main(void) {
	char buf[513];
	char *prefix, *cmd, *params[MAX_IRC_PARAMS]; 
	int params_size, ret;
	int i;
	size_t len;
	
	while (1) {
		fgets(buf, 512, stdin);
		len = strlen(buf);
		if (len > 0) {
			buf[len-1] = '\r';
			buf[len] = '\n';
			buf[len+1] = '\0';
		}
		printf("Buffer contents:\n");
		prtbuf(buf);
		ret = parse_msg(buf, len+1, &prefix, &cmd, params, &params_size);
		if (ret == -1) {
			printf("Invalid message!\n");
		}
		else {
			printf("Prefix: %s\n", ret == 1 ? prefix : "<no prefix>");
			printf("Command: %s\n", cmd);
			for (i = 0; i < params_size; i++) {
				printf("Parameter %d: %s\n", i+1, params[i]);
			}
		}
	}
	
	return 0;
}
*/
