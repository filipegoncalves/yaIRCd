#include "serverinfo.h"
#include "msgio.h"
#include "send_rpl.h"

/** @file
	@brief Functions that send a reply to a command issued by an IRC user
	
	This file provides a set of functions to send various replies to an IRC user in the sequence of a command sent to the server.
	
	@author Filipe Goncalves
	@date November 2013
*/

/** Sends MOTD to a client.
   @param client The client to send MOTD to.
   @todo Make this configurable, including the server name.
 */
void send_motd(struct irc_client *client)
{
	const char *format =
		":%s " RPL_MOTDSTART " %s :- %s Message of the day - \r\n"
		":%s " RPL_MOTD " %s :- Hello, welcome to this IRC server.\r\n"
		":%s " RPL_MOTD " %s :- This is an experimental server with very few features implemented.\r\n"
		":%s " RPL_MOTD " %s :- Only PRIVMSG is allowed at the moment, sorry!\r\n"
		":%s " RPL_MOTD
		" %s :- A team of highly trained monkeys has been dispatched to deal with this unpleasant situation.\r\n"
		":%s " RPL_MOTD
		" %s :- For now, there's really nothing you can do besides guessing who's online and PRIVMSG'ing them.\r\n"
		":%s " RPL_MOTD " %s :- Good luck! :P\r\n"
		":%s " RPL_ENDOFMOTD " %s :End of /MOTD command\r\n";
	yaircd_send(client, format,
		    get_server_name(), client->nick, get_server_name(),
		    get_server_name(), client->nick,
		    get_server_name(), client->nick,
		    get_server_name(), client->nick,
		    get_server_name(), client->nick,
		    get_server_name(), client->nick,
		    get_server_name(), client->nick,
		    get_server_name(), client->nick);
}

/** Sends the welcome message to a newly registred user
   @param client The client to greet.
   @todo Make this correct to present settings properly
 */
void send_welcome(struct irc_client *client)
{
	const char *format =
		":%s " RPL_WELCOME " %s :Welcome to the Internet Relay Network %s!%s@%s\r\n"
		":%s " RPL_YOURHOST " %s :Your host is %s, running version %s\r\n"
		":%s " RPL_CREATED " %s :This server was created %s\r\n"
		":%s " RPL_MYINFO " %s :%s %s %s %s\r\n";

	yaircd_send(client, format,
		    get_server_name(), client->nick, client->nick, client->username, client->hostname,
		    get_server_name(), client->nick, get_server_name(), YAIRCD_VERSION,
		    get_server_name(), client->nick, __DATE__ " " __TIME__,
		    get_server_name(), client->nick, get_server_name(), YAIRCD_VERSION, "UMODES=xTR", "CHANMODES=mvil");
}

/** Sends a generic PRIVMSG command notification to a given target. The destination can either be a channel or a user.
   @param from The message's author
   @param to Message's recipient. This can be the other end of a private conversation, or it can be a regular channel
      user receiving a message on a channel.
   @param dest The destination of the message. If it is a private conversation, it will just be `to`'s nickname;
      otherwise, it is the channel name.
   @param msg The message to deliver.
 */
void notify_privmsg(struct irc_client *from, struct irc_client *to, char *dest, char *msg)
{
	char message[MAX_MSG_SIZE + 1];
	const char *format =
		":%s!%s@%s PRIVMSG %s :%s\r\n";
	cmd_print_reply(message, sizeof(message), format, from->nick, from->username, from->realname, dest, msg);
	client_enqueue(&to->write_queue, message);
	ev_async_send(to->ev_loop, &to->async_watcher);
}
