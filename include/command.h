#ifndef COMMAND_H
#define COMMAND_H

/*
 * Receive a user message and normalize it, trimming carriage returns and
 * using spaces as a delimiter to extract commands and tokens to dispatch
 * to their respective handler functions.
 */
extern void handle_user_msg(int sender_fd, char *buf);

#endif
