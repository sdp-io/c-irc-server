#ifndef NETWORK_H
#define NETWORK_H

#include <poll.h>
#include <stddef.h>

extern char *inet_ntop2(void *addr, char *buf, size_t size);

extern int get_listener_socket(void);

extern void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count,
                        int *fd_size);

extern void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

extern void handle_new_connection(int listener, int *fd_count, int *fdsize,
                                  struct pollfd **pfds);

extern void handle_client_data(int listener, int *fd_count, struct pollfd *pfds,
                               int *pfd_i);

extern void process_connections(int listener, int *fd_count, int *fd_size,
                                struct pollfd **pfds);

#endif
