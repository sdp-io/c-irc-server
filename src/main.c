#include "network.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Helper function that handles the initialization of the set of polling file
 * descriptors via malloc, the initialization of the listener socket, as well as
 * the placement of the subsequent listener socket into the set of polling file
 * descriptors.
 */
int init_pfds(struct pollfd **pfds, int *fd_size, int *fd_count,
              int *listener) {
  // Begin with enough room for 5 total connections
  // When capacity is reached, will realloc()
  *fd_size = 5;
  *fd_count = 0;

  *pfds = malloc(sizeof **pfds * (*fd_size));

  // Set up and get a listening socket
  *listener = get_listener_socket();

  if (*listener == -1) {
    return -1;
  }

  // Add the listener to the set of connections.
  // Report ready to read when receiving an incoming connection
  (*pfds)[0].fd = *listener;
  (*pfds)[0].events = POLLIN;

  *fd_count = 1; // Increment to account for the listener.

  return 0;
}

/*
 * Main: Create a listener socket and a set of file descriptors to poll.
 * Then, loop forever polling/processing connections.
 */
int main(void) {
  int listener; // Listening socket file descriptor
  int fd_size;  // Capacity for pfds
  int fd_count; // Total fds currently in pfds
  struct pollfd *pfds;

  if (init_pfds(&pfds, &fd_size, &fd_count, &listener) == -1) {
    fprintf(stderr, "main: error getting listening socket\n");
    exit(EXIT_FAILURE);
  }

  puts("pollserver: waiting for connections...");

  // Main polling loop
  for (;;) {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1) {
      perror("main: poll");
      exit(EXIT_FAILURE);
    }

    // Iterate through connections within the set of poll fds looking for data
    // to read
    process_connections(listener, &fd_count, &fd_size, &pfds);
  }

  free(pfds);
}
