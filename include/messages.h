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

// TODO: Add documentation
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
