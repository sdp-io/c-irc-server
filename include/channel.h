#ifndef CHANNEL_H
#define CHANNEL_H

#include "structs.h"

/*
 * Search for a channel based on a provided channel name string. If the
 * channel does not exist, create it. Ensure that the calling user is not
 * already active within the specified channel's user list and add them to the
 * list of users currently active in the channel. Updates the output_channel
 * parameter with the searched or newly created channel. Returns 0 on success,
 * -1 on failure.
 */
extern int join_channel(struct User *joining_user, char *channel_name,
                        struct Channel **output_channel);

/*
 * Handles logic for the PART command. If channel is not found, or user is
 * not active within the channel format and send ERR_NOSUCHCHANNEL or
 * ERR_NOTONCHANNEL. Otherwise, remove the user from the channel's user list and
 * notify the remaining users that the parting user has left the channel along
 * with an optional parting message provided by the parting user.
 */
extern int handle_part_channel_logic(struct User *parting_user,
                                     struct Channel *target_channel,
                                     char *parting_message);

/*
 * Removes a user from a channel's user list and subsequently updates the user
 * list.
 */
extern int channel_remove_user(struct Channel *channel_users,
                               struct User *parting_user);

/*
 * Iterates through the list of channels currently active on the server.
 * Searches the list for the provided target Channel struct to delete.
 * Once found, frees all memory allocated for the Channel struct and its
 * ChannelNode container struct and removes it from the list of currently active
 * channels.
 */
extern int delete_channel(struct Channel *target_channel);

// TODO: Add documentation
extern struct Channel *get_channel(char *channel_name);

/*
 * Retrieves the container node for a specified user or NULL if they are not in
the channel. Used for checking the query user's channel modes.
*/
extern struct UserNode *channel_get_member(struct Channel *target_channel,
                                           struct User *query_user);

// TODO: Add documentation
extern void channel_message_users(struct Channel *target_channel, char *message,
                                  int exclude_fd);

// TODO: Add documentation
extern bool channel_has_user(struct Channel *target_channel,
                             struct User *query_user);

// TODO: Add documentation
extern void channel_remove_topic(struct Channel *target_channel);

// TODO: Add documentation
extern void channel_set_topic(struct Channel *target_channel, char *new_topic);

/*
 * Returns the first active channel on the IRC server or NULL if it does not
 * exist. Used to begin iteration of channels currently on the server.
 */
extern struct Channel *channel_get_head(void);

/*
 * Retrieves the next active channel within the list of channels currently on
 * the IRC server. Returns NULL if the end of the list has been reached.
 */
extern struct Channel *channel_get_next(struct Channel *current_channel);

/*
 * Returns a printable string of the target channel's current topic. If the
 * channel does not have a currently set topic, returns an empty string.
 */
extern char *channel_get_topic(struct Channel *target_channel);

#endif
