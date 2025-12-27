#include "user.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "9034" // Defined port that we will be listening on

/*
 * Convert an IPv4 or IPv6 socket to a readable presentation string.
 */
const char *inet_ntop2(void *addr, char *buf, size_t size) {
  // Initialize sockaddr_storage for passed socket param, as well as for IPv4
  // and IPv6 types
  struct sockaddr_storage *sas = addr;
  struct sockaddr_in *sa4;
  struct sockaddr_in6 *sa6;
  void *src;

  // Retrieve the socket address depending on its type (IPv4 or IPv6)
  switch (sas->ss_family) {
  case AF_INET:
    sa4 = addr;
    src = &(sa4->sin_addr);
    break;
  case AF_INET6:
    sa6 = addr;
    src = &(sa6->sin6_addr);
    break;
  default:
    return NULL;
  }

  // Return the presentation string depending on socket type
  return inet_ntop(sas->ss_family, src, buf, size);
}

/*
 * Return a listening socket.
 */
int get_listener_socket(void) {
  int listener; // Descriptor for the listening socket
  int yes = 1;  // Flag for setsocktopt() SO_REUSEADDR
  int rv;

  struct addrinfo hints, *ai, *p;

  // Clear hints to remove any trash data, then specify hints params
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // Set listening socket family to type IPv4
  hints.ai_socktype = SOCK_STREAM; // Set listening socket type to stream (TCP)
  hints.ai_flags = AI_PASSIVE; // Instruct getaddrinfo() to use our own IP addr

  // Attempt to retrieve the local addr based on hints, allocating results to
  // the ai struct
  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  // Iterate through the retrieved set of addresses provided by getaddrinfo()
  for (p = ai; p != NULL; p = p->ai_next) {
    // Check if the currently observed address is a valid socket
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue; // Current socket is invalid
    }

    // Prevent "addr already in use" error from occurring when PORT and socket
    // are available
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // Attempt to bind to the currently observed address
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener); // Unable to bind, move to next address
      continue;
    }

    break; // Successfully bound to listener socket
  }

  // Unable to retrieve set of addresses from getaddrinfo() to use as a bound
  if (p == NULL) {
    return -1;
  }

  freeaddrinfo(ai); // Finished with set of addresses provided by getaddrinfo()

  // Begin listening to listener socket
  if (listen(listener, 10) == -1) {
    return -1;
  }

  return listener;
}

/*
 * Add a new file descriptor to the set of poll file descriptor
 */
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size) {
  // If the poll file descriptor set is at max capacity, allocate more space
  if (*fd_count == *fd_size) {
    *fd_size *= 2; // Double the current size
    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read status
  (*pfds)[*fd_count].revents = 0;

  (*fd_count)++;
}

/*
 * Add a new file descriptor to the set of poll file descriptor.
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  // Copy the last file descriptor to the current index and decrement fd_count
  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)--;
}

/*
 * Handle new incoming connections.
 */
void handle_new_connection(int listener, int *fd_count, int *fd_size,
                           struct pollfd **pfds) {
  struct sockaddr_storage remoteaddr; // Incoming address
  socklen_t addrlen;
  int newfd; // Newly accepted socket descriptor
  char remoteIP[INET6_ADDRSTRLEN];

  addrlen = sizeof(remoteaddr);
  newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

  if (newfd == -1) {
    perror("accept");
  } else {
    // Add to users list, currently no NICK for user so set to NULL
    if (add_to_users(newfd, NULL) == -1) {
      fprintf(stderr, "Server full or OOM, rejecting connection from %d\n",
              newfd);
      close(newfd); // Close attempted connection as user malloc failed
      return;

      // TODO: Send IRC compliant error message to users that fail to connect
      // due to reasons unrelated to malloc failure.
    }

    // Valid fd and successfully added to users, therefore add to pfds
    add_to_pfds(pfds, newfd, fd_count, fd_size);

    printf("pollserver: new connection from %s on socket %d\n",
           inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP), newfd);
  }
}

/*
 * Handle the receiving of regular client data OR client hangups.
 */
void handle_client_data(int listener, int *fd_count, struct pollfd *pfds,
                        int *pfd_i) {
  char buf[256]; // Buffer for the receiving of client data

  int sender_fd = pfds[*pfd_i].fd;

  int nbytes = recv(sender_fd, buf, sizeof buf, 0);

  if (nbytes <= 0) { // Received error, or the client has closed the connection
    if (nbytes == 0) {
      // Client has closed the connection
      printf("pollserver: socket %d has hung up\n", sender_fd);
    } else {
      // Received error
      perror("recv");
    }

    // Close the client socket and remove from pollfds
    close(sender_fd);
    del_from_pfds(pfds, *pfd_i, fd_count);

    // Decrement the pfd iterator to rexamine the slot we just deleted
    (*pfd_i)--;
  } else { // Received some good data from the client
    printf("pollserver: recv from fd %d: %.*s", sender_fd, nbytes, buf);
    // Send the received data to everyone else within the pfds
    for (int j = 0; j < *fd_count; j++) {
      int destination_fd = pfds[j].fd;

      // Do not send the data to the listener and sender of the data
      if (destination_fd != listener && destination_fd != sender_fd) {
        if (send(destination_fd, buf, nbytes, 0) == -1) {
          perror("send");
        }
      }
    }
  }
}

/*
 * Process/poll all existing connections currently within the set of pollfds
 */
void process_connections(int listener, int *fd_count, int *fd_size,
                         struct pollfd **pfds) {
  for (int i = 0; i < *fd_count; i++) {
    // Check if someone is ready to be read
    if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
      // Found one!

      if ((*pfds)[i].fd == listener) {
        // If the listener is ready to read, we are receiving a new connection.
        handle_new_connection(listener, fd_count, fd_size, pfds);
      } else {
        // Else it is a client sending data
        handle_client_data(listener, fd_count, *pfds, &i);
      }
    }
  }
}
