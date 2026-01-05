#ifndef USER_H
#define USER_H

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
extern int set_user_nick(int sender_fd, char *sender_nick);

/*
 * Handle the setting of a user's username on the server. Handles the sending of
 * the numeric reply ERR_ALREADYREGISTERED when the sending user has a
 * pre-existing username and real name. Allocates memory to the newly
 * provided username and real name and assigns them to the corresponding User
 * struct for the sending user.
 */
extern void set_user_username(int sender_fd, char *user_param, char *mode_param,
                              char *realname_param);

/*
 * Return the User struct associated with the provided file descriptor.
 */
extern struct User *get_user(int query_fd);

#endif
