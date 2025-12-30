#include "structs.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

      // Free allocated memory associated with the deleted user
      free(users_iterator->user_info->nick);
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

/*
 * Handle the setting of a user's nickname on the server. Iterates through
 * the list of active users on the server to ensure nickname availability.
 * Frees the memory allocated to the user's previous nickname (if applicable)
 * then allocates memory to the new nickname and assigns it to the user's
 * corresponding User struct
 */
int set_user_nick(int sender_fd, char *sender_nick) {
  if (!is_valid_nick(sender_nick)) {
    return -1;
  }

  // User struct to capture the user after list iteration
  struct User *sender = NULL;
  struct User *iterator_user = NULL; // Track user at each list iteration

  struct UserNode *users_iterator = users_head;
  while (users_iterator != NULL) {
    // Unpack user_info from current UserNode to make comparisons
    iterator_user = users_iterator->user_info;

    // Check if current iterators user is same as sender
    if (iterator_user->user_fd == sender_fd) {
      sender = iterator_user;
    }

    // Prevent null comparisons with uninitialized active users in the user list
    if (iterator_user->nick == NULL) {
      users_iterator = users_iterator->next;
      continue;
    }

    // Compare current iterations user nick to nick param given by sender
    if ((strcmp(iterator_user->nick, sender_nick)) == 0) {
      return -1; // TODO: Replace with RFC compliant error code
    }

    users_iterator = users_iterator->next;
  }

  // If sender has a pre-existing nick, free it.
  if (sender->nick != NULL) {
    free(sender->nick);
  }

  // Allocate memory for the new nick
  sender->nick = strdup(sender_nick);
  return 0;
}

/*
 * Receive a user message and normalize it, trimming carriage returns and
 * using spaces as a delimiter to extract commands and tokens to dispatch
 * to their respective handler functions.
 */
void handle_user_msg(int sender_fd, char *buf) {
  char *user_cmd = strtok(buf, " \r\n");

  // Check if only received carriage return from user
  if (user_cmd == NULL) {
    return;
  }

  if ((strcasecmp(user_cmd, "NICK")) == 0) {
    char *nick_param = strtok(NULL, " \r\n");
    set_user_nick(sender_fd, nick_param);
  }
}
