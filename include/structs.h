#ifndef STRUCTS_H
#define STRUCTS_H

#include "hashmap.h"
#include <stdbool.h>

#define BUF_SIZE 512

#define MOTD_LINE_SIZE 100

#define MAX_NICK_LEN 30

#define SERVER_NAME "localhost"

#define SERVER_INFO "Insert server information here!"

extern char *oper_password;

/*
 * Struct containing information for a user currently on the IRC server.
 * Allows for ease of nickname verification and messaging capabilities.
 */
struct User {
  struct Channel *joined_channels; // Hash table for user's active channels
  char user_buf[BUF_SIZE + 1];
  char *nick;
  char *host_name;
  char *user_name;
  char *real_name;
  char *away_msg;
  int user_fd; // Will also be used as a key for the hash table
  int buf_len;
  bool has_username;
  bool has_nick;
  bool is_registered;
  bool is_oper;
  bool is_away;
  bool is_dead;           // Marks user to be removed from pfds and cleaned up
  UT_hash_handle hh;      // Make this struct hashable
  UT_hash_handle hh_nick; // Hashable handle for user nick
};

/*
 * Struct which acts as a container for the User struct.
 * Allows for a linked list implementation to traverse, retrieve, and modify
 * users that are currently active on the IRC server.
 */
struct UserNode {
  struct User *user_info;
  struct UserNode *next;
  bool channel_op;
  bool channel_voice;
};

/*
 * Struct which acts as a container for the Channel struct.
 * Allows for a linked list implementation to traverse, retrieve, and modify
 * channels that are currently active on the IRC server.
 */
struct ChannelNode {
  struct Channel *channel_info;
  struct ChannelNode *next;
};

/*
 * Struct containing information for a channel on the IRC server.
 * Allows for creation of channels along with channel management and user
 * interaction.
 */
struct Channel {
  int total_users;
  char *channel_name; // Uses format "#channel"
  char *topic;
  struct UserNode *user_list;
  UT_hash_handle hh_global; // Hash handle for all channels on the server
  UT_hash_handle hh_user;   // Hash handle for channels in a user's channel list
};

#endif
