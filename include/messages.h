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

#endif
