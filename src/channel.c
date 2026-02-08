#include "messages.h"
#include "network.h"
#include "structs.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static struct ChannelNode *channels_head = NULL;

// When a channel is JOIN'd that does not exist, this function is called to
// create it.
struct Channel *create_channel(char *channel_name) {
  struct Channel *new_channel = malloc(sizeof(struct Channel));
  if (new_channel == NULL) {
    fprintf(stderr,
            "create_channel: error allocating memory for new channel\n");
    return NULL;
  }

  char *new_channel_name = strdup(channel_name);
  if (new_channel_name == NULL) {
    fprintf(stderr,
            "create_channel: error allocating memory for new channel name\n");
    free(new_channel);
    return NULL;
  }

  new_channel->channel_name = new_channel_name;
  new_channel->user_list = NULL;
  new_channel->topic = NULL;

  struct ChannelNode *new_channel_node = malloc(sizeof(struct ChannelNode));
  if (new_channel_node == NULL) {
    fprintf(stderr,
            "create_channel: error allocating memory for new channel node\n");
    free(new_channel_name);
    free(new_channel);
    return NULL;
  }

  new_channel_node->channel_info = new_channel;
  new_channel_node->next = channels_head;

  channels_head = new_channel_node;

  return new_channel;
}

struct Channel *get_channel(char *channel_name) {
  struct ChannelNode *channel_node_iterator = channels_head;

  while (channel_node_iterator != NULL) {
    struct Channel *current_channel = channel_node_iterator->channel_info;
    char *current_channel_name = current_channel->channel_name;

    if (strcasecmp(current_channel_name, channel_name) == 0) {
      return current_channel;
    }

    channel_node_iterator = channel_node_iterator->next;
  }

  return NULL;
}

bool channel_has_user(struct UserNode *user_list, struct User *query_user) {
  int query_user_fd = query_user->user_fd;

  struct UserNode *user_node_iterator = user_list;
  while (user_node_iterator != NULL) {
    int current_user_fd = user_node_iterator->user_info->user_fd;
    if (query_user_fd == current_user_fd) {
      return true;
    }

    user_node_iterator = user_node_iterator->next;
  }

  return false;
}

int channel_add_user(struct Channel *channel, struct User *new_user) {
  struct UserNode *new_user_node = malloc(sizeof(struct UserNode));
  if (new_user_node == NULL) {
    fprintf(stderr,
            "channel_add_user: error allocating memory for new user node\n");
    return -1;
  }

  new_user_node->user_info = new_user;
  new_user_node->next = channel->user_list;

  // Insert new user node to head of the user list
  channel->user_list = new_user_node;

  return 0;
}

void channel_message_users(struct UserNode *user_list, char *message) {
  if (user_list == NULL) {
    return;
  }

  struct UserNode *user_node_iterator = user_list;
  while (user_node_iterator != NULL) {
    int iterator_user_fd = user_node_iterator->user_info->user_fd;
    send_string(iterator_user_fd, message, strlen(message));

    user_node_iterator = user_node_iterator->next;
  }
}

int channel_remove_user(struct UserNode **channel_users,
                        struct User *parting_user) {
  // Handle cases where the users list is NULL or empty
  if (channel_users == NULL || *channel_users == NULL) {
    return -1;
  }

  // Create iterator variables to traverse the channel's users linked list
  struct UserNode *user_node_iterator = *channel_users;
  struct User *iterator_user = user_node_iterator->user_info;
  struct UserNode *prev_user_node = NULL;

  int parting_user_fd = parting_user->user_fd;
  int iterator_user_fd = iterator_user->user_fd;

  // Handle removal of user at head of the linked list
  if (parting_user_fd == iterator_user_fd) {
    *channel_users = user_node_iterator->next;
    free(user_node_iterator);
    return 0;
  }

  while (user_node_iterator != NULL) {
    iterator_user = user_node_iterator->user_info;
    iterator_user_fd = iterator_user->user_fd;

    // Remove and free the matching user from the channel's user list
    if (parting_user_fd == iterator_user_fd) {
      prev_user_node->next = user_node_iterator->next;
      free(user_node_iterator);
      return 0;
    }

    prev_user_node = user_node_iterator;
    user_node_iterator = user_node_iterator->next;
  }

  // Parting user's file descriptor not found in channel's list of active users
  return -2;
}

// JOIN a channel. Add user to list of joined users.
int join_channel(struct User *joining_user, char *channel_name) {
  struct Channel *searched_channel = get_channel(channel_name);

  // Searched channel not found in list of channels so create it
  if (searched_channel == NULL) {
    searched_channel = create_channel(channel_name);
  }

  // Failed to create channel, log and return error
  if (searched_channel == NULL) {
    fprintf(stderr, "join_channel: failed to create channel %s\n",
            channel_name);
    return -1;
  }

  struct UserNode *channel_user_list = searched_channel->user_list;
  if (channel_has_user(channel_user_list, joining_user)) {
    // User already active in the channel, return normally
    return 0;
  }

  int join_status = channel_add_user(searched_channel, joining_user);
  if (join_status == -1) {
    char *joining_user_nick = joining_user->nick;
    fprintf(stderr, "join_channel: failed to add user %s to %s user list\n",
            joining_user_nick, channel_name);
    return -1;
  }

  return 0;
}

int leave_channel(struct User *parting_user, char *channel_name,
                  char *parting_message) {

  struct Channel *searched_channel = get_channel(channel_name);

  // Searched channel not found, return as failure
  if (searched_channel == NULL) {
    // TODO: Handle ERR_NOSUCHCHANNEL reply
    printf("TEST - ERR_NOSUCHCHANNEL\n");
    return -1;
  }

  struct UserNode **channel_users = &(searched_channel->user_list);
  if (!channel_has_user(*channel_users, parting_user)) {
    // TODO: Handle ERR_NOTONCHANNEL reply
    printf("TEST - ERR_NOTONCHANNEL\n");
    return -1;
  }

  if (parting_message == NULL) {
    // TODO: Handle sending of parting msg to users in channel
    parting_message = "";
  }

  char parting_msg_buf[BUF_SIZE];
  char *parting_user_nick = parting_user->nick;
  char *parting_user_username = parting_user->user_name;
  char *parting_user_host = parting_user->host_name;

  format_reply(parting_msg_buf, BUF_SIZE, FMT_PART, parting_user_nick,
               parting_user_username, parting_user_host, channel_name,
               parting_message);

  // Send message to every user within the channel user list (including sender)
  channel_message_users(*channel_users, parting_msg_buf);

  // Then attempt to remove sender from the channel user list
  int part_status = channel_remove_user(channel_users, parting_user);
  if (part_status == -1) {
    char *parting_user_nick = parting_user->nick;
    fprintf(stderr,
            "leave_channel: channel %s has NULL user list while user %s tried "
            "to part\n",
            channel_name, parting_user_nick);
    return -1;
  } else if (part_status == -2) {
    char *parting_user_nick = parting_user->nick;
    fprintf(stderr, "leave_channel: user %s not found in channel %s\n",
            parting_user_nick, channel_name);
    return -1;
  }

  return 0;
}
