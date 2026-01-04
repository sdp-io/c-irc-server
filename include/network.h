#ifndef NETWORK_H
#define NETWORK_H

#include <poll.h>
#include <stddef.h>
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
extern void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count,
                        int *fd_size);

/*
 * Removes a file descriptor at index i from the set of file descriptors
 * currently being polled by replacing it with the last element and
 * decrementing the count by one.
 */
extern void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

/*
 * Handle new incoming connections. Adds the new connection to the set of file
 * descriptor being polled as well as adding the connection to the list of users
 * currently active on the server.
 */
extern void handle_new_connection(int listener, int *fd_count, int *fdsize,
                                  struct pollfd **pfds);

/*
 * Handle the receiving of regular client data OR client hangups. On client
 * hangups, removes the client from the list of file descriptors being polled,
 * as well as from the list of users currently active on the server.
 */
extern void handle_client_data(int listener, int *fd_count, struct pollfd *pfds,
                               int *pfd_i);

/*
 * Process/poll all existing connections currently within the set of pollfds
 */
extern void process_connections(int listener, int *fd_count, int *fd_size,
                                struct pollfd **pfds);

/*
 * Given a buffer containing a corresponding numeric reply (RFC 2812),
 * attempt to send the reply to the specified file descriptor.
 */
extern void send_numeric_reply(int sender_fd, char *reply_buf, u_int size);

#endif
