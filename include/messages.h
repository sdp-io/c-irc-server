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
 * 221 RPL_UMODEIS
 * Sent as a reply to a user querying their own global modes (e.g., "MODE
 * mynick").
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (the user requesting the info)
 *  3. The Mode String (e.g., "+o", "+a", or just "+" if user has no modes)
 */
#define RPL_UMODEIS ":%s 221 %s :%s\r\n"

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
 * 301 RPL_AWAY
 * Sent as a response to a PRIVMSG command being sent to a target that is
 * currently away.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Recipient Nickname
 *  3. Recipient's Away Message
 */
#define RPL_AWAY ":%s 301 %s %s :%s\r\n"

/*
 * 305 RPL_UNAWAY
 * Sent to a client to confirm that they are no longer marked as being away.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define RPL_UNAWAY ":%s 305 %s :You are no longer marked as being away\r\n"

/*
 * 306 RPL_NOWAWAY
 * Sent to a client to confirm that they have been successfully marked as being
 * away.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define RPL_NOWAWAY ":%s 306 %s :You have been marked as being away\r\n"

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
 * 315 RPL_ENDOFWHO
 * Sent after all RPL_WHOREPLY replies have been sent. Used to mark the end of
 * the WHO list.
 *
 * Format Args:
 *  1. Sever Name
 *  2. Recipient's Nickname
 *  3. The mask used in the WHO query
 */
#define RPL_ENDOFWHO ":%s 315 %s %s :End of WHO list\r\n"

/*
 * 318 RPL_ENDOFWHOIS
 * Marks the end of the WHOIS response sequence.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Query Nickname
 */
#define RPL_ENDOFWHOIS ":%s 318 %s :End of WHOIS list\r\n"

/*
 * 322 RPL_LIST
 * Sent as a response to the LIST command to provide information about all
 * channels on the server, or a singular channel on the server.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (The user requesting the info)
 *  3. Channel Name
 *  4. Number of users in the channel
 *  5. Channel topic
 */
#define RPL_LIST ":%s 322 %s %s %d :%s\r\n"

/*
 * 323 RPL_LISTEND
 * Sent to indicate the end of a LIST response sequence
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define RPL_LISTEND ":%s 323 %s :End of LIST\r\n"

// TODO: 331 RPL_NOTOPIC Add documentation
#define RPL_NOTOPIC ":%s 331 %s :No topic is set\r\n"

// TODO: 332 RPL_TOPIC Add documentation
#define RPL_TOPIC ":%s 332 %s :%s\r\n"

/*
 * 352 RPL_WHOREPLY
 * Sent as a reply to the WHO command. Each matching user generates one reply.
 *
 * Format Args:
 *  1. Server Name
 *  2. Recipient Nickname
 *  3. Channel Name
 *  4. Target Username
 *  5. Target Hostname
 *  6. Target Server Name
 *  7. Target Nickname
 *  8. Target's Status ("H" for here, "G" for away, "@" for channel op, "+" for
 *     channel voice mode)
 *  9. Hop Count (Hardcoded as 0 as this is a single server)
 *  10. Target's Realname
 */
#define RPL_WHOREPLY ":%s 352 %s %s %s %s %s %s %s :%d %s\r\n"

/*
 * 353 RPL_NAMREPLY
 * Sent as a reply to the NAMES command. Contains a space-separated list of
 * nicknames currently within a channel. As the server does not currently
 * support private or secret channels, '=' is hardcoded into the reply to
 * signify the target channel is public.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (the user requesting the list of names)
 *  3. Channel Name
 *  4. Space-separated list of nicknames (with '=' prefix currently hardcoded)
 */
#define RPL_NAMREPLY ":%s 353 %s = %s :%s\r\n"

/*
 * 366 RPL_ENDOFNAMES
 * Sent to mark the end of the RPL_NAMREPLY sequence of replies.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Channel Name
 */
#define RPL_ENDOFNAMES ":%s 366 %s %s :End of NAMES list\r\n"

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
 * 381 RPL_YOUREOP
 * Sent back to a client that has successfully used the OPER command
 * and gained global operator status
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define RPL_YOUREOP ":%s 381 %s :You are now an IRC operator\r\n"

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
 * 403 ERR_NOSUCHCHANNEL
 * Returned when a command refers to a channel name that does not exist.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The invalid channel name
 */
#define ERR_NOSUCHCHANNEL ":%s 403 %s %s :No such channel\r\n"

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
 * 442 ERR_NOTONCHANNEL
 * Returned when a client tries to perform a channel-based command
 * on a channel they are not currently a member of.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The channel name
 */
#define ERR_NOTONCHANNEL ":%s 442 %s %s :You're not on that channel\r\n"

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
 * 461 ERR_NEEDMOREPARAMS
 * Returned by the server to indicate that the command sent by the
 * client did not contain enough parameters.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. The command name that failed
 */
#define ERR_NEEDMOREPARAMS ":%s 461 %s %s :Not enough parameters\r\n"

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
 * 464 ERR_PASSWDMISMATCH
 * Returned to indicated a failed attempt at using the OPER command
 * due to an incorrect password.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define ERR_PASSWDMISMATCH ":%s 464 %s :Password incorrect\r\n"

/*
 * 502 ERR_USERSDONTMATCH
 * Sent by the server to a user who is trying to view or change the
 * user mode for a user other than themself.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (the user trying the command)
 */
#define ERR_USERSDONTMATCH ":%s 502 %s :Cannot change mode for other users\r\n"

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
 * FMT_TOPIC
 * Used to relay the changing of a channel's topic from a sender to all users
 * active within that channel
 *
 * Format Args:
 *  1. Sender Nickname
 *  2. Sender Username
 *  3. Sender Hostname
 *  4. Target Channel
 *  5. Contents of the new topic
 */
#define FMT_TOPIC ":%s!%s@%s TOPIC %s :%s\r\n"

/*
 * FMT_PART
 * Used to relay a response upon the PARTing of a user from a specified channel.
 * Relays an optional PARTing string to users remaining in the channel.
 *
 * Format Args:
 *  1. Sender Nickname
 *  2. Sender Username
 *  3. Sender Hostname
 *  4. Target Channel
 *  5. (Optional) Contents of the parting message
 */
#define FMT_PART ":%s!%s@%s PART %s :%s\r\n"

/*
 * FMT_MODE
 * Used to broadcast a successful mode change to clients.
 * For channel modes, this is sent to everyone in the channel.
 * For user modes, this is echoed back to the calling user who changed their
 * mode.
 *
 * Format Args:
 *  1. Sender Nickname
 *  2. Sender Username
 *  3. Sender Hostname
 *  4. Target (Channel name or User nickname)
 *  5. Mode Flag String (e.g., "+o", "-o")
 *  6. Member Parameter (e.g., the target nickname for channel mode changes.
 *     Pass an empty string "" if the mode doesn't take a parameter).
 */
#define FMT_MODE ":%s!%s@%s MODE %s %s %s\r\n"

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

/*
 * FMT_QUIT
 * Used to relay the quitting of a client from the server. Sends a custom quit
 * message, or by default, the quitting client's nickname.
 *
 * Format Args:
 *   1. Quitter Nickname
 *   2. Quitter Username
 *   3. Quitter Hostname
 *   4. Quitting Message (Quitter's Nick by Default)
 */
#define FMT_QUIT ":%s!%s@%s QUIT :%s\r\n"

#endif
