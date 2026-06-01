#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <sys/epoll.h>
#include <sys/types.h>

/*
 * Convert an IPv4 or IPv6 socket to a readable presentation string.
 */
extern char *inet_ntop2(void *addr, char *buf, size_t size);

/*
 * Return a listening socket for the server to receive connections and client
 * data.
 */
extern int get_listener_socket(void);

/*
 * Add a new file descriptor to the set of file descriptors being polled.
 */
extern void add_to_epfd(int epfd, int new_fd);

/*
 * Removes a file descriptor from the set of file descriptors being polled.
 */
extern void del_from_epfd(int epfd, int old_fd);

/*
 * Handle new incoming connections. Adds the new connection to the set of file
 * descriptor being polled as well as adding the connection to the list of users
 * currently active on the server.
 */
extern void handle_new_connection(int listener, int epfd);

/*
 * Handle the receiving of regular client data OR client hangups. On client
 * hangups, removes the client from the list of file descriptors being polled,
 * as well as from the list of users currently active on the server.
 */
extern void handle_client_data(int sender_fd, int epfd);

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
extern int send_string(int sender_fd, char *reply_buf, u_int size);

#endif
