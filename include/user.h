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
 * Updates a user's nickname, allocating memory to it and adding it to the user
 * nick hashtable. Frees memory for any pre-existing nicknames and removes it
 * from the hashtable before assigning the new nickname.
 */
extern void set_user_nick(struct User *sender_user, char *sender_nick);

/*
 * Updates a user's username and realname, allocating memory to it as well as
 * setting the user's has_username field to true.
 */
extern void set_user_username(struct User *sender_user, char *user_param,
                              char *realname_param);

/*
 * Compares the list of joined channels for the requester user with the joined
 * channels of the candidate user. If the two users share a common channel,
 * returns true. Otherwise, returns false.
 */
extern bool users_share_channel(struct User *requester, struct User *candidate);

/*
 * Returns the first user with a NICK on the IRC server or NULL if it does not
 * exist. Used to begin iteration of users with a nick currently on the server.
 */
extern struct User *user_get_head(void);

/*
 * Retrieves the next user within the list of users with a NICK currently on the
 * IRC server. Returns NULL if the end of the list has been reached.
 */
extern struct User *user_get_next(struct User *current_user);

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
extern char *user_get_buf(int user_fd);

/*
 * Grabs the User struct associated with a provided user's file descriptor
 * and returns the length of the user's corresponding buffer.
 */
extern int user_get_buf_len(int user_fd);

/*
 * Returns the total number of operators currently active on the server.
 */
extern int user_get_oper_count(void);

/*
 * Grabs the User struct associated with a provided user's file descriptor
 * and sets the length of the user's corresponding buffer to a new length.
 */
void user_set_buf_len(int user_fd, int new_len);

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
 * Sets a user's registration status to true, and updates the unknown and
 * registered user counts on the server.
 */
extern void user_set_registered(struct User *target_user);

/*
 * Allocates memory for a ChannelNode struct to contain the provided channel and
 * add it to the user's list of currently joined channels.
 */
extern void user_add_channel(struct User *user, struct Channel *new_channel);

/*
 * Sets a user's operator status and modifies the server's operator user count.
 * If the user is given operator status (status param is true), the operator
 * user count is incremented. If the user's operator status is revoked (status
 * param is false), the operator user count is decremented.
 */
extern void user_set_operator_status(struct User *target_user, bool status);

/*
 * Sets a user's away status based on a provided away message parameter.
 * If the parameter is not NULL, sets the user's away message and status.
 * If the message parameter is NULL, revokes the user's away message and status.
 */
extern bool user_set_away_status(struct User *target_user, char *away_msg);

/*
 * Searches the list for a channel, and then removes it from the user's list of
actively joined channels, freeing all memory associated with it.
*/
extern void user_remove_channel(struct User *user,
                                struct Channel *target_channel);

/*
 * Iterates through each of the user's actively joined channels and parts from
 * them without a parting message.
 */
extern void user_remove_all(struct User *user);

#endif
