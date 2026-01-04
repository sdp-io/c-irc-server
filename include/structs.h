#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>

#define BUF_SIZE 512

#define MAX_NICK_LEN 30

#define SERVER_NAME "localhost"

/*
 * 001 RPL_WELCOME
 * Sent upon successful registration of a client (both NICK and USER commands
 * ran successfully.)
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 *  3. Client Nickname
 *  4. Client Username
 *  5. Client Hostname
 */
#define RPL_WELCOME                                                            \
  ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n"

/*
 * 432 ERR_ERRONEOUSNICKNAME
 * Returned when a NICK command contains invalid characters, length, or format.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (or "*" if unset)
 *  3. The invalid nickname
 */
#define ERR_ERRONEOUSNICKNAME ":%s 432 %s %s :Erroneous nickname\r\n"

/*
 * 433 ERR_NICKNAMEINUSE
 * Returned when a NICK command attempts to assign a name that is already in use
 * by another user.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname (or "*" if unset)
 *  3. The unavailable nickname
 */
#define ERR_NICKNAMEINUSE ":%s 433 %s %s :Nickname is already in use\r\n"

/*
 * 462 ERR_ALREADYREGISTERED
 * Returned when a client attempts to call the USER command after client
 * registration has already been completed.
 *
 * Format Args:
 *  1. Server Name
 *  2. Target Nickname
 */
#define ERR_ALREADYREGISTERED                                                  \
  ":%s 462 %s :Unauthorized command (already registered)\r\n"

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
