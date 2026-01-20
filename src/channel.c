#include "structs.h"
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

  int join_status = channel_add_user(searched_channel, joining_user);
  if (join_status == -1) {
    char *joining_user_nick = joining_user->nick;
    fprintf(stderr, "join_channel: failed to add user %s to %s user list\n",
            joining_user_nick, channel_name);
    return -1;
  }

  return 0;
}
