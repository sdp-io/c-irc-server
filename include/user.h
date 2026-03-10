#ifndef USER_H
#define USER_H

#include "structs.h"

/*
 * Given a user file descriptor and string denoting the user's nickname,
 * allocates memory for a User struct with the provided information and adds it
 * to the list of users currently active on the IRC server.
 *
 * Returns 0 on success and -1 on failure.
 */
extern int add_to_users(int user_fd, char *user_host);

/*
 * Given a user file descriptor to delete, searches the list of users currently
 * active on the IRC server for the matching file descriptor. Once found, frees
 * the memory allocated to the corresponding user and decrements the count of
 * users currently active on the server.
 */
extern void del_from_users(int user_fd);

/*
 * Handle the setting of a user's nickname on the server. Iterates through
 * the list of active users on the server to ensure nickname availability.
 * Frees the memory allocated to the user's previous nickname (if applicable)
 * then allocates memory to the new nickname and assigns it to the user's
 * corresponding User struct.
 *
 * Returns 0 on success and -1 on failure.
 */
extern int set_user_nick(int user_fd, char *sender_nick);

/*
 * Handle the setting of a user's username on the server. Handles the sending of
 * the numeric reply ERR_ALREADYREGISTERED when the sending user has a
 * pre-existing username and real name. Allocates memory to the newly
 * provided username and real name and assigns them to the corresponding User
 * struct for the sending user.
 */
extern void set_user_username(int user_fd, char *user_param, char *mode_param,
                              char *realname_param);

/*
 * Return the User struct associated with the provided file descriptor.
 */
extern struct User *get_user_by_fd(int query_fd);

/*
 * Return the User struct associated with the provided nickname.
 */
extern struct User *get_user_by_nick(char *query_nick);

/*
 * Grabs the User struct associated with a provided user's file descriptor
 * and returns the user's corresponding buffer.
 */
extern char *get_user_buf(int user_fd);

/*
 * Grabs the User struct associated with a provided user's file descriptor
 * and returns the length of the user's corresponding buffer.
 */
int get_user_buf_len(int user_fd);

/*
 * Grabs the User struct associated with a provided user's file descriptor
 * and sets the length of the user's corresponding buffer to a new length.
 */
void set_user_buf_len(int user_fd, int new_len);

/*
 * Return the number of unregistered users currently
 * connected to the server.
 */
extern int get_unknown_user_count(void);

/*
 * Return the number of registered users currently connected to the server.
 */
extern int get_registered_user_count(void);

/*
 * Allocates memory for a ChannelNode struct to contain the provided channel and
 * add it to the user's list of currently joined channels.
 */
extern int user_add_channel(struct User *user, struct Channel *new_channel);

/*
 * Sets a user's operator status to true
 */
extern void user_set_oper(struct User *user);

#endif
