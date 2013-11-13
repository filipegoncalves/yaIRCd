#ifndef __PROTOCOL_SPECS_GUARD__
#define __PROTOCOL_SPECS_GUARD__

/** @file
	@brief IRC Protocol definitions and limits
	
	This file is a set of macros that are related to the protocol requirements.
	Definitions in this file include:
	<ul>
		<li>Every limit that arises from the IRC protocol specification (max. message size, max. nickname length, etc.)</li>
		<li>Error replies and command replies are defined as unsigned constants.</li>
	</ul>
	
	See RFC 1459 for further information.
	
	@author Filipe Goncalves
	@date November 2013
*/

/* Protocol limits */
/** Max. message size, including terminating \\r\\n, as specified in Section 2.3 */
#define MAX_MSG_SIZE ((size_t) 512)

/** Max. number of parameters in an IRC message, as specified in Section 2.3 */
#define MAX_IRC_PARAMS 15

/** Max. nick length, as specified in RFC Section 1.2 */
#define MAX_NICK_LENGTH 9
/* End protocol limits */

/* Error replies */
/** Define the width of a numeric reply */
#define NUMREPLY_WIDTH ((size_t) 3)

/** Used to indicate the nickname parameter supplied to a command is currently unused. */
#define ERR_NOSUCHNICK "401"

/** Used to indicate the server name given currently doesn't exist. */
#define ERR_NOSUCHSERVER "402"

/** Used to indicate the given channel name is invalid. */
#define ERR_NOSUCHCHANNEL "403"

/** Sent to a user who is either (a) not on a channel which is mode +n or (b) not a chanop (or mode +v) on a channel which has mode +m set and is trying to send a PRIVMSG message to that channel. */
#define ERR_CANNOTSENDTOCHAN "404"

/** Sent to a user when they have joined the maximum number of allowed channels and they try to join another channel. */
#define ERR_TOOMANYCHANNELS "405"

/** Returned by WHOWAS to indicate there is no history information for that nickname. */
#define ERR_WASNOSUCHNICK "406"

/** Returned to a client which is attempting to send PRIVMSG/NOTICE using the user\@host destination format and for a user\@host which has several occurrences. */
#define ERR_TOOMANYTARGETS "407"

/** PING or PONG message missing the originator parameter which is required since these commands must work without valid prefixes. */
#define ERR_NOORIGIN "409"

/** Returned to a client who issued a PRIVMSG or SUMMON command without a recipient */
#define ERR_NORECIPIENT "411"

/** Returned to a client who issued a PRIVMSG command without any text to send */
#define ERR_NOTEXTTOSEND "412"

/** Returned to a client who issued a PRIVMSG command with a mask that does not contain a top level domain */
#define ERR_NOTOPLEVEL "413"

/** 412 - 414 are returned by PRIVMSG to indicate that the message wasn't delivered for some reason. ERR_NOTOPLEVEL and ERR_WILDTOPLEVEL are errors that are returned when an invalid use of "PRIVMSG $<server>" or "PRIVMSG #<host>" is attempted. */
#define ERR_WILDTOPLEVEL "414"

/** Returned to a registered client to indicate that the command sent is unknown by the server. */
#define ERR_UNKNOWNCOMMAND "421"

/** Server's MOTD file could not be opened by the server. */
#define ERR_NOMOTD "422"

/** Returned by a server in response to an ADMIN message when there is an error in finding the appropriate information. */
#define ERR_NOADMININFO "423"

/** Generic error message used to report a failed file operation during the processing of a message. */
#define ERR_FILEERROR "424"

/** Returned when a nickname parameter expected for a command and isn't found. */
#define ERR_NONICKNAMEGIVEN "431"

/** Returned after receiving a NICK message which contains characters which do not fall in the defined set. See Section 2.3.1 for details on valid nicknames. */
#define ERR_ERRONEUSNICKNAME "432"

/** Returned when a NICK message is processed that results in an attempt to change to a currently existing nickname. */
#define ERR_NICKNAMEINUSE "433"

/** Returned by a server to a client when it detects a nickname collision (registered of a NICK that already exists by another server). */
#define ERR_NICKCOLLISION "436"

/** Returned by the server to indicate that the target user of the command is not on the given channel. */
#define ERR_USERNOTINCHANNEL "441"

/** Returned by the server whenever a client tries to perform a channel effecting command for which the client isn't a member. */
#define ERR_NOTONCHANNEL "442"

/** Returned by the summon after a SUMMON command for a user was unable to be performed since they were not logged in. */
#define ERR_NOLOGIN "444"

/** Returned as a response to the SUMMON command. Must be returned by any server which does not implement it. */
#define ERR_SUMMONDISABLED "445"

/** Returned as a response to the USERS command. Must be returned by any server which does not implement it. */
#define ERR_USERSDISABLED "446"

/** Returned by the server to indicate that the client must be registered before the server will allow it to be parsed in detail. */
#define ERR_NOTREGISTERED "451"

/** Returned by the server by numerous commands to indicate to the client that it didn't supply enough parameters. */
#define ERR_NEEDMOREPARAMS "461"

/** Returned by the server to any link which tries to change part of the registered details (such as password or user details from second USER message). */
#define ERR_ALREADYREGISTRED "462"

/** Returned to a client which attempts to register with a server which does not been setup to allow connections from the host the attempted connection is tried. */
#define ERR_NOPERMFORHOST "463"

/** Returned to indicate a failed attempt at registering a connection for which a password was required and was either not given or incorrect. */
#define ERR_PASSWDMISMATCH "464"

/** Returned after an attempt to connect and register yourself with a server which has been setup to explicitly deny connections to you. */
#define ERR_YOUREBANNEDCREEP "465"

/** Error sent to a client who tries to set a password for a channel that already has one */
#define ERR_KEYSET "467"

/** Returned to a client who tries to join a channel that is full */
#define ERR_CHANNELISFULL "471"

/** Error sent to a client when an unknown mode use is attempted */
#define ERR_UNKNOWNMODE "472"

/** Returned to a client who tried to join an invite-only chan without having been invited */
#define ERR_INVITEONLYCHAN "473"

/** Error sent to a client who tried to join a channel from which he is banned */
#define ERR_BANNEDFROMCHAN "474"

/** Returned when a client tries to join a key protected channel without providing a password, or providing a wrong password */
#define ERR_BADCHANNELKEY "475"

/** Any command requiring operator privileges to operate must return this error to indicate the attempt was unsuccessful. */
#define ERR_NOPRIVILEGES "481"

/** Any command requiring 'chanop' privileges (such as MODE messages) must return this error if the client making the attempt is not a chanop on the specified channel. */
#define ERR_CHANOPRIVSNEEDED "482"

/** Any attempts to use the KILL command on a server are to be refused and this error returned directly to the client. */
#define ERR_CANTKILLSERVER "483"

/** If a client sends an OPER message and the server has not been configured to allow connections from the client's host as an operator, this error must be returned. */
#define ERR_NOOPERHOST "491"

/** Returned by the server to indicate that a MODE message was sent with a nickname parameter and that the a mode flag sent was not recognized. */
#define ERR_UMODEUNKNOWNFLAG "501"

/** Error sent to any user trying to view or change the user mode for a user other than themselves.  */
#define ERR_USERSDONTMATCH "502"
/* End error replies */

/* Command responses */
/** Dummy reply number. Not used. */
#define RPL_NONE 300U

/** Reply format used by USERHOST to list replies to the query list. */
#define RPL_USERHOST 302U

/** Reply format used by ISON to list replies to the query list. */
#define RPL_ISON 303U

/** Reply after setting aray */
#define RPL_AWAY 301U

/** Reply after unsetting away */
#define RPL_UNAWAY 305U

/** These replies are used with the AWAY command (if allowed). RPL_AWAY is sent to any client sending a PRIVMSG to a client which is away. RPL_AWAY is only sent by the server to which the client is connected. Replies RPL_UNAWAY and RPL_NOWAWAY are sent when the client removes and sets an AWAY message. */
#define RPL_NOWAWAY 306U

/** WHOIS command reply */
#define RPL_WHOISUSER 311U

/** WHOIS (server) command reply */
#define RPL_WHOISSERVER 312U

/** WHOIS Operator command reply */
#define RPL_WHOISOPERATOR 313U

/** WHOIS idle reply */
#define RPL_WHOISIDLE 317U

/** WHOIS end reply */
#define RPL_ENDOFWHOIS 318U

/** Replies 311 - 313, 317 - 319 are all replies generated in response to a WHOIS message. Given that there are enough parameters present, the answering server must either formulate a reply out of the above numerics (if the query nick is found) or return an error reply. The '*' in RPL_WHOISUSER is there as the literal character and not as a wild card. For each reply set, only RPL_WHOISCHANNELS may appear more than once (for long lists of channel names). The purpose of the RPL_WHOISCHANNELS messages are to enumerate all the channels of which a user is a member. The '@' and '+' characters next to the channel name indicate whether a client is a channel operator or has been granted permission to speak on a moderated channel. The RPL_ENDOFWHOIS reply is used to mark the end of processing a WHOIS message. */
#define RPL_WHOISCHANNELS 319U

/** Similar to whois, but for whowas */
#define RPL_WHOWASUSER 314U

/** When replying to a WHOWAS message, a server must use the replies RPL_WHOWASUSER, RPL_WHOISSERVER or ERR_WASNOSUCHNICK for each nickname in the presented list. At the end of all reply batches, there must be RPL_ENDOFWHOWAS (even if there was only one reply and it was an error). */
#define RPL_ENDOFWHOWAS 369U

/** Start of list reply */
#define RPL_LISTSTART 321U

/** End of list reply */
#define RPL_LIST 322U

/** Replies RPL_LISTSTART, RPL_LIST, RPL_LISTEND mark the start, actual replies with data and end of the server's response to a LIST command. If there are no channels available to return, only the start and end reply must be sent.  */
#define RPL_LISTEND 323U

/** Channel modes reply */
#define RPL_CHANNELMODEIS 324U

/** No topic set reply */
#define RPL_NOTOPIC 331U

/** When sending a TOPIC message to determine the channel topic, one of two replies is sent. If the topic is set, RPL_TOPIC is sent back else RPL_NOTOPIC.  */
#define RPL_TOPIC 332U

/** Returned by the server to indicate that the attempted INVITE message was successful and is being passed onto the end client. */
#define RPL_INVITING 341U

/** Returned by a server answering a SUMMON message to indicate that it is summoning that user. */
#define RPL_SUMMONING 342U

/** Reply by the server showing its version details. The \<version\> is the version of the software being used (including any patchlevel revisions) and the \<debuglevel\> is used to indicate if the server is running in "debug mode". The "comments" field may contain any comments about the version or further version details. */
#define RPL_VERSION 351U

/** WHO reply */
#define RPL_WHOREPLY 352U

/** The RPL_WHOREPLY and RPL_ENDOFWHO pair are used to answer a WHO message. The RPL_WHOREPLY is only sent if there is an appropriate match to the WHO query. If there is a list of parameters supplied with a WHO message, a RPL_ENDOFWHO must be sent after processing each list item with \<name\> being the item. More info on RFC. */
#define RPL_ENDOFWHO 315U

/** Reply to a NAMES message; see RPL_ENDOFNAMES */
#define RPL_NAMREPLY 353U

/** To reply to a NAMES message, a reply pair consisting of RPL_NAMREPLY and RPL_ENDOFNAMES is sent by the server back to the client. If there is no channel found as in the query, then only RPL_ENDOFNAMES is returned. The exception to this is when a NAMES message is sent with no parameters and all visible channels and contents are sent back in a series of RPL_NAMEREPLY messages with a RPL_ENDOFNAMES to mark the end. */
#define RPL_ENDOFNAMES 366U

/** LINKS command reply */
#define RPL_LINKS 364U

/** In replying to the INKS message, a server must send replies back using the RPL_LINKS numeric and mark the end of the list using an RPL_ENDOFLINKS reply. */
#define RPL_ENDOFLINKS 365U

/** Bans list */
#define RPL_BANLIST 367U

/** When listing the active 'bans' for a given channel, a server is required to send the list back using the RPL_BANLIST and RPL_ENDOFBANLIST messages. A separate RPL_BANLIST is sent for each active banid. After the banids have been listed (or if none present) a RPL_ENDOFBANLIST must be sent. */
#define RPL_ENDOFBANLIST 368U

/** For INFO command */
#define RPL_INFO 371U

/** A server responding to an INFO message is required to send all its 'info' in a series of RPL_INFO messages with a RPL_ENDOFINFO reply to indicate the end of the replies. */
#define RPL_ENDOFINFO 374U

/** Signalizes start of MOTD message */
#define RPL_MOTDSTART 375U

/** Used when MOTD is being sent */
#define RPL_MOTD 372U

/** When responding to the MOTD message and the MOTD file is found, the file is displayed line by line, with each line no longer than 80 characters, using RPL_MOTD format replies. These should be surrounded by a RPL_MOTDSTART (before the RPL_MOTDs) and an RPL_ENDOFMOTD (after). */
#define RPL_ENDOFMOTD 376U

/** RPL_YOUREOPER is sent back to a client which has just successfully issued an OPER message and gained operator status. */
#define RPL_YOUREOPER 381U

/** If the REHASH option is used and an operator sends a REHASH message, an RPL_REHASHING is sent back to the operator. */
#define RPL_REHASHING 382U

/** When replying to the TIME message, a server must send the reply using the RPL_TIME format above. The string showing the time need only contain the correct day and time there. There is no further requirement for the time string. */
#define RPL_TIME 391U

/** Start of USERS command output */
#define RPL_USERSSTART 392U

/** Result of USERS command */
#define RPL_USERS 393U

/** End of output for USERS command */
#define RPL_ENDOFUSERS 394U

/** If the USERS message is handled by a server, the replies RPL_USERSTART, RPL_USERS, RPL_ENDOFUSERS and RPL_NOUSERS are used. RPL_USERSSTART must be sent first, following by either a sequence of RPL_USERS or a single RPL_NOUSER. Following this is RPL_ENDOFUSERS. */
#define RPL_NOUSERS 395U

/** Intermediate servers must return a RPL_TRACELINK reply to indicate that the TRACE passed through it and where its going next */
#define RPL_TRACELINK 200U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACECONNECTING 201U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACEHANDSHAKE 202U

/** Invalid TRACE */
#define RPL_TRACEUNKNOWN 203U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACEOPERATOR 204U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACEUSER 205U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACESERVER 206U

/** Used for TRACE command. See RFC for more info. */
#define RPL_TRACENEWTYPE 208U

/** The RPL_TRACE* are all returned by the server in response to the TRACE message. How many are returned is dependent on the the TRACE message and whether it was sent by an operator or not. There is no predefined order for which occurs first. Replies RPL_TRACEUNKNOWN, RPL_TRACECONNECTING and RPL_TRACEHANDSHAKE are all used for connections which have not been fully established and are either unknown, still attempting to connect or in the process of completing the 'server handshake'. RPL_TRACELINK is sent by any server which handles a TRACE message and has to pass it on to another server. The list of RPL_TRACELINKs sent in response to a TRACE command traversing the IRC network should reflect the actual connectivity of the servers themselves along that path. RPL_TRACENEWTYPE is to be used for any connection which does not fit in the other categories but is being displayed anyway. */
#define RPL_TRACELOG 261U

/** For /stats */
#define RPL_STATSLINKINFO 211U

/** Reply of command count in stats */
#define RPL_STATSCOMMANDS 212U

/** clines stats reply */
#define RPL_STATSCLINE 213U

/** nlines stats reply */
#define RPL_STATSNLINE 214U

/** ilines stats reply */
#define RPL_STATSILINE 215U

/** klines stats reply */
#define RPL_STATSKLINE 216U

/** yline stats reply */
#define RPL_STATSYLINE 218U

/** End of stats reply */
#define RPL_ENDOFSTATS 219U

/** lline stats reply */
#define RPL_STATSLLINE 241U

/** uptime information reply for /stats command */
#define RPL_STATSUPTIME 242U

/** reply to report current O:Lines to /stats */
#define RPL_STATSOLINE 243U

/** hlines stats reply */
#define RPL_STATSHLINE 244U

/** To answer a query about a client's own mode, RPL_UMODEIS is sent back. */
#define RPL_UMODEIS 221U

/** Reply to LUSER command. Counts number of users, number of invisible users, and number of servers. */
#define RPL_LUSERCLIENT 251U

/** Replies back how many operators are online to LUSER command */
#define RPL_LUSEROP 252U

/** Reports number of unknown connections to LUSER command */
#define RPL_LUSERUNKNOWN 253U

/** Reports how many channels are formede to LUSER command */
#define RPL_LUSERCHANNELS 254U

/** In processing an LUSERS message, the server sends a set of replies from RPL_LUSERCLIENT, RPL_LUSEROP, RPL_USERUNKNOWN, RPL_LUSERCHANNELS and RPL_LUSERME. When replying, a server must send back RPL_LUSERCLIENT and RPL_LUSERME. The other replies are only sent back if a non-zero count is found for them. */
#define RPL_LUSERME 255U

/** Server administrative info. reply */
#define RPL_ADMINME 256U

/** General details about the server. See RPL_ADMINEMAIL. */
#define RPL_ADMINLOC1 257U

/** More specific server details. See RPL_ADMINEMAIL. */
#define RPL_ADMINLOC2 258U

/** When replying to an ADMIN message, a server is expected to use replies RLP_ADMINME through to RPL_ADMINEMAIL and provide a text message with each. For RPL_ADMINLOC1 a description of what city, state and country the server is in is expected, followed by details of the university and department (RPL_ADMINLOC2) and finally the administrative contact for the server (an email address here is required) in RPL_ADMINEMAIL.  */
#define RPL_ADMINEMAIL 259U
/* End command responses */

#endif /* __PROTOCOL_SPECS_GUARD__ */