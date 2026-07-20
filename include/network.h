#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <sys/epoll.h>
#include <sys/types.h>

/*
 * Return a listening socket for the server to receive connections and client
 * data.
 */
extern int get_listener_socket(void);

/*
 * Processes the type of event received from the polled file descriptos and
 * routes it to its appropriate function.
 */
extern void process_connections(int listener, int epfd,
                                struct epoll_event *event);

/*
 * Given a buffer containing a corresponding reply (RFC 2812)
 * or message, attempt to send it to the specified file descriptor.
 * Returns 0 if the message was sent successfully and -1 if it fails.
 */
extern int send_string(int sender_fd, char *reply_buf, size_t size);

#endif
