#ifndef MESSAGES_H
#define MESSAGES_H

/*
 * 001 RPL_WELCOME
 * Sent upon successful registration of a client (both NICK and USER commands
 * ran successfully.)
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Client Nickname
 *  4. Client Username
 *  5. Client Hostname
 */
#define RPL_WELCOME                                                            \
  ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n"

/*
 * 251 RPL_LUSERCLIENT
 * Sent as part of the LUSERS response, detailing the global number of
 * users, services, and servers on the network.
 *
 * Format Args:
 *  1. Server Name
 *  2. Number of Users (int)
 *  3. Number of Services (int)
 *  4. Number of Servers (int)
 */
#define RPL_LUSERCLIENT                                                        \
  ":%s 251 :There are %d users and %d services on %d servers\r\n"

/*
 * 252 RPL_LUSEROP
 * Sent as part of the LUSERS response to indicate the number of IRC operators.
 *
 * Format Args:
 *  1. Server Name
 *  2. Number of Operators (int)
 */
#define RPL_LUSEROP ":%s 252 %d :operator(s) online\r\n"

/*
 * 253 RPL_LUSERUNKNOWN
 * Sent as part of the LUSERS response to indicate the number of unknown
 * (unregistered) connections currently on the server.
 *
 * Format Args:
 *  1. Server Name
 *  2. Number of Unknown Connections (int)
 */
#define RPL_LUSERUNKNOWN ":%s 253 %d :unknown connection(s)\r\n"

/*
 * 254 RPL_LUSERCHANNELS
 * Sent as part of the LUSERS response to indicate the number of channels
 * currently formed on the server.
 *
 * Format Args:
 *  1. Server Name
 *  2. Number of Channels (int)
 */
#define RPL_LUSERCHANNELS ":%s 254 %d :channels formed\r\n"

/*
 * 255 RPL_LUSERME
 * Sent as part of the LUSERS response, detailing the number of clients
 * and servers connected specifically to the local server.
 *
 * Format Args:
 *  1. Server Name
 *  2. Number of Local Clients (int)
 *  3. Number of Local Servers (int)
 */
#define RPL_LUSERME ":%s 255 :I have %d clients and %d servers\r\n"

/*
 * 311 RPL_WHOISUSER
 * One of the replies to the WHOIS command. Contains the specific
 * user information for the target nickname.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (The user requesting the info)
 *  3. Query Nickname (The user being looked up)
 *  4. Query Username
 *  5. Query Hostname
 *  6. Query Realname
 */
#define RPL_WHOISUSER ":%s 311 %s %s %s * :%s\r\n"

/*
 * 312 RPL_WHOISSERVER
 * One of the replies to the WHOIS command. Indicates which server
 * the queried user is currently connected to.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Query Nickname
 *  4. Server Hostname (where Query Nick is connected)
 *  5. Server Info String
 */
#define RPL_WHOISSERVER ":%s 312 %s %s :%s\r\n"

/*
 * 318 RPL_ENDOFWHOIS
 * Marks the end of the WHOIS response sequence.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Query Nickname
 */
#define RPL_ENDOFWHOIS ":%s 318 %s :End of /WHOIS list\r\n"

/*
 * 372 RPL_MOTD
 * Contains one line of the Message of the Day text.
 * Sent repeatedly for every line in the MOTD file.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. One line of MOTD text
 */
#define RPL_MOTD ":%s 372 :- %s\r\n"

/*
 * 375 RPL_MOTDSTART
 * Sent at the beginning of the MOTD response sequence.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Server Name (to display in the text)
 */
#define RPL_MOTDSTART ":%s 375 :- %s Message of the day - \r\n"

/*
 * 376 RPL_ENDOFMOTD
 * Sent at the end of the MOTD response sequence to indicate
 * no more text will follow.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define RPL_ENDOFMOTD ":%s 376 :End of MOTD command\r\n"

/*
 * 401 ERR_NOSUCHNICK
 * Returned to indicate that the nickname parameter supplied to
 * command could not be found within the list of active users.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The nickname that could not be found
 */
#define ERR_NOSUCHNICK ":%s 401 %s %s :No such nick/channel\r\n"

/*
 * Returned when a PONG command is sent without a corresponding server/message
 * parameter.
 *
 * Format Args:
 *  1. Server Name
 */
#define ERR_NOORIGIN ":%s 409 :No origin specified\r\n"

/*
 * 411 ERR_NORECIPIENT
 * Returned when a command that requires a recipient/target (such as PRIVMSG)
 * receives NULL instead of a valid recipient.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The command that was called without a recipient
 */
#define ERR_NORECIPIENT ":%s 411 %s :No recipient given %s\r\n"

/*
 * 412 ERR_NOTEXTTOSEND
 * Returned when a command that requires text to send receives NULL instead.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define ERR_NOTEXTTOSEND ":%s 412 %s :No text to send\r\n"

/*
 * 421 ERR_UNKNOWNCOMMAND
 * Returned when the server receives a command that is not in its
 * list of implemented handlers.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The unknown command string
 */
#define ERR_UNKNOWNCOMMAND ":%s 421 %s %s :Unknown command\r\n"

/*
 * 432 ERR_ERRONEOUSNICKNAME
 * Returned when a NICK command contains invalid characters, length, or format.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (or "*" if unset)
 *  3. The invalid nickname
 */
#define ERR_ERRONEOUSNICKNAME ":%s 432 %s %s :Erroneous nickname\r\n"

/*
 * 433 ERR_NICKNAMEINUSE
 * Returned when a NICK command attempts to assign a name that is already in use
 * by another user.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (or "*" if unset)
 *  3. The unavailable nickname
 */
#define ERR_NICKNAMEINUSE ":%s 433 %s %s :Nickname is already in use\r\n"

/*
 * 451 ERR_NOTREGISTERED
 * Returned by the server to indicate that the client
 * MUST be registered before the server will allow it
 * to be parsed in detail.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (or "*" if unset)
 */
#define ERR_NOTREGISTERED ":%s 451 %s :You have not registered\r\n"

/*
 * 462 ERR_ALREADYREGISTERED
 * Returned when a client attempts to call the USER command after client
 * registration has already been completed.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define ERR_ALREADYREGISTERED                                                  \
  ":%s 462 %s :Unauthorized command (already registered)\r\n"

/*
 * FMT_PRIVMSG
 * Used to relay a private message from a sender to a target recipient.
 *
 * Format Args:
 *   1. Sender Nickname
 *   2. Sender Username
 *   3. Sender Hostname
 *   4. Target Recipient
 *   5. Contents of the message
 */
#define FMT_PRIVMSG ":%s!%s@%s PRIVMSG %s :%s\r\n"

/*
 * FMT_PRIVMSG
 * Used to relay a notice from a sender to a target recipient.
 *
 * Format Args:
 *   1. Sender Nickname
 *   2. Sender Username
 *   3. Sender Hostname
 *   4. Target Recipient
 *   5. Contents of the message
 */
#define FMT_NOTICE ":%s!%s@%s NOTICE %s :%s\r\n"

/*
 * FMT_PING
 * Used to relay a response to a PING command to a target recipient.
 *
 * Format Args:
 *   1. Server Name
 *   2. Server Name
 *   3. Contents of the original PING message
 */
#define FMT_PING ":%s PONG %s %s\r\n"

#endif
