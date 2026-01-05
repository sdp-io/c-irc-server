#include "messages.h"
#include "network.h"
#include "structs.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Linked list of users for the IRC server
static struct UserNode *users_head = NULL;

// Count of users currently active on the IRC Server
static int user_count = 0;

struct User *get_user_by_fd(int query_fd) {
  struct UserNode *users_iterator = users_head;
  struct User *iterator_user = NULL;

  while (users_iterator != NULL) {
    iterator_user = users_iterator->user_info;

    if (iterator_user->user_fd == query_fd) {
      return iterator_user;
    }

    users_iterator = users_iterator->next;
  }
  // Queried user not found
  return NULL;
}

struct User *get_user_by_nick(char *query_nick) {
  struct UserNode *users_iterator = users_head;
  struct User *iterator_user = NULL;
  char *iterator_user_nick = NULL;

  while (users_iterator != NULL) {
    iterator_user = users_iterator->user_info;
    iterator_user_nick = iterator_user->nick;

    // Skip all iterations with NULL nicknames
    if (iterator_user_nick == NULL) {
      users_iterator = users_iterator->next;
      continue;
    }

    if (strcasecmp(query_nick, iterator_user_nick) == 0) {
      return iterator_user;
    }

    users_iterator = users_iterator->next;
  }

  // Queried user not found
  return NULL;
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
    return -1;
  }

  new_user->nick = NULL;
  new_user->host_name = host_name;
  new_user->user_name = NULL;
  new_user->real_name = NULL;
  new_user->user_fd = user_fd;
  new_user->has_nick = false;
  new_user->has_username = false;
  new_user->is_registered = false;

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
      free(users_iterator->user_info->user_name);
      free(users_iterator->user_info->real_name);
      free(users_iterator->user_info->host_name);
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

int set_user_nick(int sender_fd, char *sender_nick) {
  // Buffer to send the corresponding numeric reply back to the sending user
  char reply_buf[BUF_SIZE];

  // If nick contains invalid characters, do not need to verify its availability
  if (!is_valid_nick(sender_nick)) {
    // Format and send ERR_ERRONEOUSNICKNAME numeric reply
    struct User *sender_user = get_user_by_fd(sender_fd);
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_ERRONEOUSNICKNAME, SERVER_NAME,
                 current_nick, sender_nick);

    send_numeric_reply(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  // User struct to capture the user after list iteration
  struct User *sender = NULL;
  struct User *iterator_user = NULL; // Track user at each list iteration

  // Get the sender's User struct from the list of active users
  // AND check if sender's nick is taken
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
    if ((strcasecmp(iterator_user->nick, sender_nick)) == 0) {
      // Specified nick is taken, format and send ERR_NICKNAMEINUSE numeric
      char *current_nick = (sender->nick != NULL) ? sender->nick : "*";

      format_reply(reply_buf, BUF_SIZE, ERR_NICKNAMEINUSE, SERVER_NAME,
                   current_nick, sender_nick);
      send_numeric_reply(sender_fd, reply_buf, strlen(reply_buf));

      return -1;
    }

    users_iterator = users_iterator->next;
  }

  // If sender has a pre-existing nick, free it.
  if (sender->nick != NULL) {
    free(sender->nick);
  }

  // Allocate memory for the new nick and update user state
  sender->nick = strdup(sender_nick);
  sender->has_nick = true;

  bool has_nick = sender->has_nick;
  bool has_username = sender->has_username;
  bool is_registered = sender->is_registered;

  // User successfully registered, change registration status and send
  // RPL_WELCOME
  if (has_nick && has_username && !is_registered) {
    sender->is_registered = true;

    char *nick = sender->nick;
    char *username = sender->user_name;
    char *host_name = sender->host_name;

    char reply_buf[BUF_SIZE];
    format_reply(reply_buf, BUF_SIZE, RPL_WELCOME, SERVER_NAME, nick, nick,
                 username, host_name);
    send_numeric_reply(sender_fd, reply_buf, strlen(reply_buf));
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
    send_numeric_reply(sender_fd, reply_buf, strlen(reply_buf));
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

    char *nick = sender_user->nick;
    char *username = sender_user->user_name;
    char *host_name = sender_user->host_name;

    char reply_buf[BUF_SIZE];
    format_reply(reply_buf, BUF_SIZE, RPL_WELCOME, SERVER_NAME, nick, nick,
                 username, host_name);
    send_numeric_reply(sender_fd, reply_buf, strlen(reply_buf));
  }
}
