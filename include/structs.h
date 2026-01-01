#ifndef STRUCTS_H
#define STRUCTS_H

#define BUF_SIZE 512

#define MAX_NICK_LEN 30

#define SERVER_NAME "localhost"

// TODO: Add documentation
#define ERR_NICKNAMEINUSE ":%s 433 %s %s :Nickname is already in use\r\n"

// TODO: Add documentation
#define ERR_ERRONEOUSNICKNAME ":%s 432 %s %s :Erroneous nickname\r\n"

/*
 * Struct containing information for a user currently on the IRC server.
 * Allows for ease of nickname verification and messaging capabilities.
 */
struct User {
  char *nick;
  int user_fd;
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
