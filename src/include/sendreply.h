#include "client.h"

/** @file
	@brief Functions that send a reply to a command issued by an IRC user
	
	This file provides a set of functions to send various replies to an IRC user in the sequence of a command sent to the server.
	
	@author Filipe Goncalves
	@date November 2013
	@see sendreply.c
*/

/* Functions documented in the source file */
void send_err_notregistered(struct irc_client *client);
void send_err_unknowncommand(struct irc_client *client, char *cmd);
void send_err_nonicknamegiven(struct irc_client *client);
void send_err_needmoreparams(struct irc_client *client, char *cmd);

