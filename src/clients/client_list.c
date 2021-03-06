#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "client_list.h"

/** @file
   @brief Client list operations implementation
   This file implements the available operations on the clients list. It is a wrapper for generic list operations; as a
      consequence, it is thread safe.
   Developers are adviced to read RFC Section 2.3.1 to learn about which characters are allowed in a nickname.
   Note that, according to RFC Section 2.2, due to IRC's scandinavian origin, the characters `{ } |` are considered to
      be the lower case equivalents of the characters `[ ] \\`, respectively.
   This is a critical issue when determining the equivalence of two nicknames.
   Every function in this file is thread safe, with the exception of `client_list_init()` and `client_list_destroy()`,
      which shall be called exactly once by the parent thread before any thread is created andafter every thread is
      dead, respectively.
   @author Filipe Goncalves
   @date November 2013
   @todo Allow digits in nicknames, as well as other special characters such as underscore
 */

/** A words list to hold every client */
static Word_list_ptr clients;

/** This function defines what characters are allowed inside a nickname. See RFC Section 2.3.1 to learn about this.
   @param s The character to check.
   @return `1` if `s` is allowed in a nickname; `0` otherwise.
 */
int nick_is_valid(char s)
{
	return
		((s >= 'a' && s <= 'z') ||
		 (s >= 'A' && s <= 'Z') || 
		 s == '-' || s == '[' || s == ']' || 
		 s == '\\' || s == '`' || s == '^' ||
		 s == '{' || s == '}' || s == '|');
}

/** Translates from an ID of a special character (a character not in `[a-z]` back to its corresponding character.
   @param i ID
   @return The special character whose ID is `i`; `-1` if `i` is an invalid ID.
 */
static inline char special_id_to_char(int i)
{
	return (i == 0 ? '-' : i == 1 ? '{' : i == 2 ? '}' : i == 3 ? '|' : i == 4 ? '`' : i == 5 ? '^' : -1);
}

/** Converts a character ID back into its corresponding character.
   @param i ID
   @return The character whose ID is `i`
 */
char nick_pos_to_char(int i)
{
	return ((char)((i < NICK_ALPHABET_SIZE) ? ('a' + i) : special_id_to_char(i - NICK_ALPHABET_SIZE)));
}

/** Translates from a special character (a character not in `[a-z]` into its ID.
   @param s A special character.
   @return `s`'s ID; `-1` if `s` is an invalid character.
 */
static inline int special_char_id(char s)
{
	return (s == '-' ? 0 : s == '[' || s == '{' ? 1 : s == ']' || s == '}' ? 2 : s == '\\' || 
			s == '|' ? 3 : s == '`' ? 4 : s == '^' ? 5 : -1);
}

/** Converts a character into its ID
   @param s The character
   @return `s`'s ID
 */
int nick_char_to_pos(char s)
{
	return (((s >= 'a' && (s) <= 'z') || (s >= 'A' && s <= 'Z')) ?
				tolower((unsigned char)s) - 'a' :
				NICK_ALPHABET_SIZE + special_char_id(s));
}

/** Initializes clients list.
   @return `0` on success; `-1` on failure. `-1` indicates a resources allocation error.
   @warning This function must be called exactly once, by the parent thread, before any thread is created and tries to
      access the list of clients.
 */
int client_list_init(void)
{
	if ((clients =
		     init_word_list(NULL, nick_is_valid, nick_pos_to_char, nick_char_to_pos, NICK_EDGES_NO)) == NULL) {
		return -1;
	}
	return 0;
}

/** Destroys a clients list after it is no longer needed.
   @warning This function must be called exactly once, by the parent thread, after every thread is dead and no more
      accesses to the list of clients will be performed.
 */
void client_list_destroy(void)
{
	destroy_word_list(clients, LIST_NO_FREE_NODE_DATA);
}

/** Finds a client by nickname, and performs an action based on that client atomically. If a client exists, the function
   provided is called to perform an arbitrary action.
   @param nick The nickname to look for; must be a null terminated characters sequence.
   @param f A pointer to a function returning a generic pointer that shall be called if a match is found. In such case,
      `f` is called with the matching client as its first parameter, and with `fargs` as second parameter.
   @param fargs This will be passed to `f` as a second parameter when a match is found and `f` is called.
   @param success After this function returns, `success` will hold `1` if `(f)()` was called, otherwise it will hold
      `0`. This allows the caller to have `(f)()` returning `NULL` and still distinguish between a successfull match and
      a nonexisting client.
   @return The result of evaluating `(f)(matching_client, fargs)`. If no client matches, `NULL` is returned.
   @warning Remember that this whole operation - search the list, find a match, call `(f)()` is performed atomically.
      Thus, `(f)()` must be fast (more clients can be waiting to read the list), but more important than that, careful
      must be taken if `(f)()` uses synchronization tools (mutexes, semaphores, etc.) to perform its job. Always
      remember that the global clients list is locked - using any locking mechanism inside `(f)()` is rarely necessary,
      and can easily introduce deadlock conditions.
   @warning `(f)()` must not call `pthread_exit()`, otherwise, the lock for this list is never unlocked, and the whole
      IRCd freezes.
   @note `(f)()` shall cast its first argument to a client's structure pointer.
 */
void *client_list_find_and_execute(char *nick, void *(*f)(void *, void *), void *fargs, int *success)
{
	return list_find_and_execute(clients, nick, f, NULL, fargs, NULL, success);
}

/** Atomically adds a client to the clients list if there isn't already a client with the same nickname.
   This operation is thread safe and guaranteed to be free of race conditions. The search and add operations are
      executed atomically.
   @param client Pointer to the new client. Cannot be `NULL`.
   @param newnick Nickname for this client.
   @return <ul>
   <li>`0` on success</li>
   <li>`LST_INVALID_NICK` if this client's nickname contains invalid characters, in which case nothing was added to the
      list
   <li>`LST_NO_MEM` if there isn't enough memory to create a new client entry</li>
   <li>`LST_ALREADY_EXISTS` if there's a known client with this nickname</li>
   </ul>
   @warning This function does not update `client->nick` to `newnick`.
   @note `newnick` is assumed to be `client`'s nickname, no matter whatever is stored in `client->nick`. This is to ease
      the task of adding new clients which may contain invalid characters in their nickname, but we haven't yet found
      out.
 */
int client_list_add(struct irc_client *client, char *newnick)
{
	return list_add(clients, (void*)client, newnick);
}

/** Deletes a client from the clients list. If no such client exists, nothing happens.
   @param client Pointer to the client that shall be deleted. Cannot be `NULL`.
 */
void client_list_delete(struct irc_client *client)
{
	(void)list_delete(clients, client->nick);
}
