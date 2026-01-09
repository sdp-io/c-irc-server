#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

/*
 * Receive a user message and normalize it, trimming carriage returns and
 * using spaces as a delimiter to extract commands and tokens to dispatch
 * to their respective handler functions.
 */
extern void handle_user_msg(int sender_fd, char *buf);

/*
 * Relays a message from the sender to the target recipient. Supports both
 * PRIVMSG and NOTICE commands. If the is_notice flag is set to true, suppresses
 * any replies that the sender may receive.
 */
extern int handle_msg_cmd(int sender_fd, char *recipient_nick, char *message,
                          bool is_notice);

/*
 * Receives a PING command from a sender along with a message parameter.
 * Formats the message into FMT_PING, then relays the PONG response back
 * to the sending user.
 */
extern int handle_ping_cmd(int sender_fd, char *message);

#endif
