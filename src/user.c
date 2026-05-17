#include "channel.h"
#include "hashmap.h"
#include "messages.h"
#include "network.h"
#include "structs.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Hash table of users for the IRC server
static struct User *users_fd_map = NULL;
static struct User *users_nick_map = NULL;

// Count of users currently connected to the IRC Server
static int unknown_user_count = 0;

// Count of users that have completed the NICK and USER commands
static int registered_user_count = 0;

// Count of users that currently have operator status on the server
static int operator_user_count = 0;

int get_unknown_user_count(void) { return unknown_user_count; }

int get_registered_user_count(void) { return registered_user_count; }

int user_get_oper_count(void) { return operator_user_count; }

void user_set_operator_status(struct User *target_user, bool status) {
  if (status) {
    target_user->is_oper = true;
    operator_user_count++;
  } else {
    target_user->is_oper = false;
    operator_user_count--;
  }
}

bool user_set_away_status(struct User *target_user, char *away_msg) {
  if (away_msg != NULL) {
    // Free pre-existing away_msg which is either a string or NULL
    free(target_user->away_msg);

    // Set user's new away message
    target_user->away_msg = strdup(away_msg);
    target_user->is_away = true;

    return true;
  }

  // No away message provided, clear current away message and UNAWAY the user
  free(target_user->away_msg);
  target_user->away_msg = NULL;
  target_user->is_away = false;

  return false;
}

struct User *user_get_head(void) { return users_nick_map; }

struct User *user_get_next(struct User *current_user) {
  if (current_user == NULL) {
    return NULL;
  }

  return current_user->hh_nick.next;
}

struct User *get_user_by_fd(int query_fd) {
  struct User *searched_user;

  HASH_FIND_INT(users_fd_map, &query_fd, searched_user);
  return searched_user;
}

struct User *get_user_by_nick(char *query_nick) {
  struct User *searched_user;

  HASH_FIND(hh_nick, users_nick_map, query_nick, strlen(query_nick),
            searched_user);

  return searched_user;
}

char *user_get_buf(int user_fd) {
  struct User *sender_user = get_user_by_fd(user_fd);
  return sender_user->user_buf;
}

int user_get_buf_len(int user_fd) {
  struct User *sender_user = get_user_by_fd(user_fd);
  return sender_user->buf_len;
}

void user_set_buf_len(int user_fd, int new_len) {
  struct User *sender_user = get_user_by_fd(user_fd);
  sender_user->buf_len = new_len;
}

void user_remove_all(struct User *target_user) {
  struct ChannelNode *current_channel, *temp;

  // Deletion safe macro
  HASH_ITER(hh, target_user->joined_channels, current_channel, temp) {
    HASH_DEL(target_user->joined_channels, current_channel);

    // Remove user from channel's active user list
    channel_remove_user(current_channel->channel_info, target_user);

    if (current_channel->channel_info->user_list == NULL) {
      delete_channel(current_channel->channel_info);
    }
  }
}

// TODO: This will always fail to send to the disconnected user, need
// alternative solution
void user_remove_channel(struct User *target_user,
                         struct Channel *target_channel) {
  struct ChannelNode *removed_channel;

  HASH_FIND(hh, target_user->joined_channels, target_channel->channel_name,
            strlen(target_channel->channel_name), removed_channel);

  HASH_DEL(target_user->joined_channels, removed_channel);

  removed_channel->channel_info = NULL;
  free(removed_channel);
}

bool users_share_channel(struct User *requester, struct User *candidate) {
  struct ChannelNode *requester_channels = requester->joined_channels;
  while (requester_channels != NULL) {
    struct ChannelNode *shared_channel = NULL;
    char *channel_name = requester_channels->channel_info->channel_name;

    HASH_FIND_STR(candidate->joined_channels, channel_name, shared_channel);

    if (shared_channel != NULL) {
      return true;
    }

    requester_channels = (struct ChannelNode *)requester_channels->hh.hh_next;
  }

  return false;
}

int add_to_users(int user_fd, char *user_host) {
  // Initialize new user to add to linked list of users
  // based on provided parameters
  struct User *new_user = malloc(sizeof(struct User));
  if (new_user == NULL) {
    fprintf(stderr, "add_to_users: error allocating memory for new user\n");
    return -1;
  }

  char *host_name = strdup(user_host);
  if (host_name == NULL) {
    fprintf(stderr,
            "add_to_users: error allocating memory for user host name\n");
    free(new_user);
    return -1;
  }

  new_user->joined_channels = NULL;
  new_user->host_name = host_name;
  new_user->user_name = NULL;
  new_user->real_name = NULL;
  new_user->nick = NULL;
  new_user->away_msg = NULL;
  new_user->user_fd = user_fd;
  new_user->buf_len = 0;
  new_user->has_nick = false;
  new_user->has_username = false;
  new_user->is_registered = false;
  new_user->is_oper = false;
  new_user->is_away = false;
  new_user->is_dead = false;
  memset(new_user->user_buf, 0, BUF_SIZE);

  // Add the new user struct to user_fd hash table
  HASH_ADD_INT(users_fd_map, user_fd, new_user);

  unknown_user_count++;

  return 0;
}

void del_from_users(int user_fd) {
  struct User *target_user = get_user_by_fd(user_fd);

  // Remove user from nick and fd hash tables
  HASH_DELETE(hh, users_fd_map, target_user);

  if (target_user->nick != NULL) {
    HASH_DELETE(hh_nick, users_nick_map, target_user);
  }

  // Disconnect user from all joined channels
  user_remove_all(target_user);

  if (target_user->is_oper) {
    operator_user_count--;
  }

  if (target_user->is_registered) {
    registered_user_count--;
  } else {
    unknown_user_count--;
  }

  free(target_user->nick);
  free(target_user->user_name);
  free(target_user->real_name);
  free(target_user->host_name);
  free(target_user);
}

// TODO: Add documentation
static void relay_nick_change(struct User *sender_user, char *old_nick,
                              char *new_nick) {
  struct ChannelNode *current_channel = sender_user->joined_channels;
  char reply_buf[BUF_SIZE];

  format_reply(reply_buf, BUF_SIZE, FMT_NICK, old_nick, sender_user->user_name,
               sender_user->host_name, new_nick);

  // Relay relevant change to the sender before relaying to channels
  send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));

  // Manual hash iteration as it is read-only
  while (current_channel != NULL) {
    channel_message_users(current_channel->channel_info, reply_buf,
                          sender_user->user_fd);

    current_channel = (struct ChannelNode *)current_channel->hh.hh_next;
  }
}

int set_user_nick(int sender_fd, char *sender_nick) {
  // Buffer to send the corresponding numeric reply back to the sending user
  char reply_buf[BUF_SIZE];
  char old_nick[MAX_NICK_LEN + 1]; // Space for null-terminator
  struct User *sender_user = get_user_by_fd(sender_fd);

  // If nick contains invalid characters, do not need to verify its availability
  if (!is_valid_nick(sender_nick)) {
    // Format and send ERR_ERRONEOUSNICKNAME numeric reply
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_ERRONEOUSNICKNAME, SERVER_NAME,
                 current_nick, sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  if (get_user_by_nick(sender_nick) != NULL) {
    // Specified nick is taken, format and send ERR_NICKNAMEINUSE numeric
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_NICKNAMEINUSE, SERVER_NAME,
                 current_nick, sender_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  // If sender has a pre-existing nick, free it and remove from the nick table
  // as well as relay the change to all of the sender's actively joined channels
  if (sender_user->nick != NULL) {
    strncpy(old_nick, sender_user->nick, MAX_NICK_LEN);
    old_nick[MAX_NICK_LEN] = '\0';

    HASH_DELETE(hh_nick, users_nick_map, sender_user);
    free(sender_user->nick);
    sender_user->nick = NULL;

    relay_nick_change(sender_user, old_nick, sender_nick);
  }

  // Allocate memory for the new nick and update user state
  sender_user->nick = strdup(sender_nick);
  sender_user->has_nick = true;

  bool has_nick = sender_user->has_nick;
  bool has_username = sender_user->has_username;
  bool is_registered = sender_user->is_registered;

  HASH_ADD_KEYPTR(hh_nick, users_nick_map, sender_user->nick,
                  strlen(sender_user->nick), sender_user);

  // User successfully registered, change registration status and send
  // RPL_WELCOME
  if (has_nick && has_username && !is_registered) {
    sender_user->is_registered = true;
    registered_user_count++;
    unknown_user_count--;

    char *nick = sender_user->nick;
    char *username = sender_user->user_name;
    char *host_name = sender_user->host_name;

    char reply_buf[BUF_SIZE];
    format_reply(reply_buf, BUF_SIZE, RPL_WELCOME, SERVER_NAME, nick, nick,
                 username, host_name);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  return 0;
}

void set_user_username(int sender_fd, char *user_param, char *mode_param,
                       char *realname_param) {
  // Verify integrity of the provided parameters
  if (user_param == NULL || mode_param == NULL || realname_param == NULL) {
    return;
  }

  struct User *sender_user = get_user_by_fd(sender_fd);

  // Sending user has a pre-existing username, format and send numeric reply
  if (sender_user->user_name != NULL || sender_user->real_name != NULL) {
    char reply_buf[BUF_SIZE];
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";
    format_reply(reply_buf, BUF_SIZE, ERR_ALREADYREGISTERED, SERVER_NAME,
                 current_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return;
  }

  // Adjust state of sending user's User struct
  sender_user->user_name = strdup(user_param);
  sender_user->real_name = strdup(realname_param);
  sender_user->has_username = true;

  bool has_nick = sender_user->has_nick;
  bool has_username = sender_user->has_username;
  bool is_registerd = sender_user->is_registered;

  // User successfully registered, change registration status and send
  // RPL_WELCOME
  if (has_nick && has_username && !is_registerd) {
    sender_user->is_registered = true;
    registered_user_count++;
    unknown_user_count--;

    char *nick = sender_user->nick;
    char *username = sender_user->user_name;
    char *host_name = sender_user->host_name;

    char reply_buf[BUF_SIZE];
    format_reply(reply_buf, BUF_SIZE, RPL_WELCOME, SERVER_NAME, nick, nick,
                 username, host_name);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }
}

void user_add_channel(struct User *user, struct Channel *new_channel) {
  struct ChannelNode *joined_channel = malloc(sizeof(struct ChannelNode));
  joined_channel->channel_info = new_channel;

  HASH_ADD_KEYPTR(hh, user->joined_channels, new_channel->channel_name,
                  strlen(new_channel->channel_name), joined_channel);
}
