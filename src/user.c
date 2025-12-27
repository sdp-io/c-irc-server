#include "structs.h"
#include <stdio.h>
#include <stdlib.h>

// Linked list of users for the IRC server
static struct UserNode *users_head = NULL;

// Count of users currently active on the IRC Server
static int user_count = 0;

int add_to_users(int user_fd, char *user_nick) {
  // Initialize new user to add to linked list of users
  // based on provided parameters
  struct User *new_user = malloc(sizeof(struct User));
  if (new_user == NULL) {
    fprintf(stderr, "add_to_users: error allocating memory for new user\n");
    return -1;
  }
  new_user->nick = user_nick;
  new_user->user_fd = user_fd;

  // Store in memory the UserNode container to store the user
  struct UserNode *new_user_node = malloc(sizeof(struct UserNode));
  if (new_user_node == NULL) {
    free(new_user);
    fprintf(stderr, "add_to_users: error allocating memory for user node\n");
    return -1;
  }
  new_user_node->user_info = new_user;
  new_user_node->next = NULL;

  // Assign new user to head of users linked list and increment user count by
  // one
  new_user_node->next = users_head;
  users_head = new_user_node;
  user_count++;

  return 0;
}
