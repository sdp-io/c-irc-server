#include "command.h"
#include "structs.h"
#include "user.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "9034" // Defined port that we will be listening on

static int BACKLOG_SIZE = 512;

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
  if (listen(listener, BACKLOG_SIZE) == -1) {
    return -1;
  }

  return listener;
}

int add_to_epfd(int epfd, int new_fd) {
  struct epoll_event ctl_event = {.events = EPOLLIN, .data = {.fd = new_fd}};
  int epoll_ctl_status = epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &ctl_event);

  return epoll_ctl_status;
}

void del_from_epfd(int epfd, int old_fd) {
  epoll_ctl(epfd, EPOLL_CTL_DEL, old_fd, NULL);
}

void handle_new_connection(int listener, int epfd) {
  struct sockaddr_storage remoteaddr; // Incoming address
  socklen_t addrlen;
  int new_fd; // Newly accepted socket descriptor
  char remoteIP[INET6_ADDRSTRLEN];

  addrlen = sizeof(remoteaddr);
  new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

  if (new_fd == -1) {
    perror("accept");
  } else {
    // Add to users list, currently no NICK for user so set to NULL
    inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP);
    if (add_to_users(new_fd, remoteIP) == -1) {
      fprintf(stderr, "Server full or OOM, rejecting connection from %d\n",
              new_fd);
      close(new_fd); // Close attempted connection as user malloc failed
      return;
    }

    // Valid fd and successfully added to users, therefore attempt to add
    if (add_to_epfd(epfd, new_fd) != -1) {
      printf("irc: new connection from %s on socket %d\n", remoteIP, new_fd);
    } else {
      printf("irc: unable to accept connection from %s on socket %d\n",
             remoteIP, new_fd);
    }
  }
}

void handle_client_data(int sender_fd, int epfd) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_buf = user_get_buf(sender_fd);
  int sender_buf_len = user_get_buf_len(sender_fd);

  int nbytes = recv(sender_fd, sender_buf + sender_buf_len,
                    BUF_SIZE - sender_buf_len, 0);

  int total_len = nbytes + sender_buf_len;

  user_set_buf_len(sender_fd, total_len);

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
    del_from_users(sender_fd);
    del_from_epfd(epfd, sender_fd);
  } else { // Received some good data from the client
    printf("pollserver: recv from fd %d: %.*s\n", sender_fd, nbytes,
           sender_buf);

    // Add a null-terminator to the next open index to create a valid string
    sender_buf[total_len] = '\0';

    // Grab end of carriage return or line feed sent by lazy clients
    char *carriage_return_ptr = strrchr(sender_buf, '\n');
    if (carriage_return_ptr != NULL) {
      // Received a complete command, begin parsing it
      int carriage_return_index = carriage_return_ptr - sender_buf;
      if (sender_buf[carriage_return_index + 1] == '\0') {
        // No fragmented data, process sender_buf directly
        handle_user_msg(sender_fd, sender_buf);

        // Start cleanup as user has QUIT the server
        if (sender_user->is_dead) {
          close(sender_fd);
          del_from_users(sender_fd);
          del_from_epfd(epfd, sender_fd);
          return;
        }

        memset(sender_buf, 0, BUF_SIZE);
        user_set_buf_len(sender_fd, 0);
      } else {
        // Fragmented data received, copy good data to temp array
        int bytes_processed = carriage_return_index + 1;

        char temp_array[bytes_processed + 1]; // Add space for terminator
        memcpy(temp_array, sender_buf, bytes_processed);
        temp_array[bytes_processed] = '\0';

        handle_user_msg(sender_fd, temp_array);

        // Start cleanup as user has QUIT the server
        if (sender_user->is_dead) {
          close(sender_fd);
          del_from_users(sender_fd);
          del_from_epfd(epfd, sender_fd);
          return;
        }

        int remaining_bytes = total_len - bytes_processed;

        // Move the unprocessed data to the front of the sender's buffer
        memmove(sender_buf, sender_buf + bytes_processed, remaining_bytes);

        user_set_buf_len(sender_fd, remaining_bytes);
      }
    }
  }
}

void process_connections(int listener, int epfd, struct epoll_event *event) {
  if (event->data.fd == listener) {
    // If the listener is ready to read, we are receiving a new connection.
    handle_new_connection(listener, epfd);
  } else {
    // Else it is a client sending data
    handle_client_data(event->data.fd, epfd);
  }
}

int send_string(int fd, char *buf, size_t size) {
  // NOTE: Ignores write attempts to closed sockets to prevent SIGPIPE signal
  // from crashing the server.
  if (send(fd, buf, size, MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "send_string: failed to send %zu bytes to fd %d\n", size,
            fd);
    return -1;
  }

  return 0;
}
