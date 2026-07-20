#include "network.h"
#include "structs.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int MAX_EVENTS = 10;

char *oper_password = NULL;

/*
 * Helper function that handles the initialization of the set of polling file
 * descriptors via malloc, the initialization of the listener socket, as well as
 * the placement of the subsequent listener socket into the set of polling file
 * descriptors.
 */
static int init_epfd(int *epfd, int *listener) {
  // Set up and get a listening socket
  *listener = get_listener_socket();

  if (*listener == -1) {
    return -1;
  }

  // Create the epoll instance and add the server to it, listening for new
  // connections
  *epfd = epoll_create1(0);

  struct epoll_event ctl_event = {.events = EPOLLIN, .data = {.fd = *listener}};
  int epoll_ctl_status = epoll_ctl(*epfd, EPOLL_CTL_ADD, *listener, &ctl_event);
  if (epoll_ctl_status == -1) {
    return -1;
  }

  return 0;
}

/*
 * Main: Create a listener socket and a set of file descriptors to poll.
 * Then, loop forever polling/processing connections.
 */
int main(int argc, char **argv) {
  int listener; // Listening socket file descriptor
  int epfd;

  // Invalid argument amount
  if (argc > 2 || argc < 2) {
    fprintf(stderr, "Invalid arguments!\n");
    exit(EXIT_FAILURE);
  }
  oper_password = argv[1];

  if (init_epfd(&epfd, &listener) == -1) {
    fprintf(stderr, "main: error getting listening socket\n");
    exit(EXIT_FAILURE);
  }

  puts("pollserver: waiting for connections...");

  // Main polling loop
  for (;;) {
    struct epoll_event events[10];
    epoll_wait(epfd, events, MAX_EVENTS, -1);

    // Iterate through connections within the set of poll fds looking for data
    // to read
    process_connections(listener, epfd, events);
  }
}
