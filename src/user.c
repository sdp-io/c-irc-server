#include "structs.h"
#include <stdio.h>
#include <stdlib.h>

// Linked list of users for the IRC server
static struct UserNode *users_head = NULL;

// Count of users currently active on the IRC Server
static int user_count = 0;

/*
 * Given a user file descriptor and string denoting the user's nickname,
 * allocates memory for a User struct with the provided information and adds it
 * to the list of users currently active on the IRC server.
 */
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

  // Assign new user to head of users linked list and increment user count
  new_user_node->next = users_head;
  users_head = new_user_node;
  user_count++;

  return 0;
}

/*
 * Given a user file descriptor to delete, searches the list of users currently
 * active on the IRC server for the matching file descriptor. Once found, frees
 * the memory allocated to the corresponding user and decrements the count of
 * users currently active on the server.
 */
void del_from_users(int user_fd) {
  struct UserNode *users_iterator = users_head;

  if (users_iterator == NULL) {
    fprintf(stderr, "del_from_users: no list to delete from!");
    return;
  }

  int iterator_fd = users_iterator->user_info->user_fd;

  // Handle removal of user at head of the linked list
  if (iterator_fd == user_fd) {
    users_head = users_iterator->next;
    free(users_iterator->user_info);
    free(users_iterator);
    user_count--;
    return;
  }

  // Traverse the linked list of active users comparing each user's fd to the
  // one that must be deleted
  struct UserNode *prev_node = users_iterator;
  users_iterator = users_iterator->next;
  while (users_iterator != NULL) {
    if (users_iterator->user_info->user_fd == user_fd) {
      prev_node->next = users_iterator->next;
      free(users_iterator->user_info);
      free(users_iterator);
      user_count--;
      return;
    }

    prev_node = users_iterator;
    users_iterator = users_iterator->next;
  }

  // The specified fd was not found within the list of user fds
  fprintf(stderr, "del_from_users: unable to delete specified user %d\n",
          user_fd);
}
