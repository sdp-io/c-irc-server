#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>

#define BUF_SIZE 512

#define MAX_NICK_LEN 30

#define SERVER_NAME "localhost"

/*
 * Struct containing information for a user currently on the IRC server.
 * Allows for ease of nickname verification and messaging capabilities.
 */
struct User {
  char *host_name;
  char *user_name;
  char *real_name;
  char *nick;
  int user_fd;
  bool has_username;
  bool has_nick;
  bool is_registered;
};

/*
 * Struct which acts as a container for the User struct.
 * Allows for a linked list implementation to traverse, retrieve, and modify
 * users that are currently active on the IRC server.
 */
struct UserNode {
  struct User *user_info;
  struct UserNode *next;
};

#endif
