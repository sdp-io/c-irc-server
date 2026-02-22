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
 * Search for a channel based on a provided channel name string and retrieve its
 * list of users active within the channel. If channel is not found, or user is
 * not active within the channel format and send ERR_NOSUCHCHANNEL or
 * ERR_NOTONCHANNEL. Otherwise, remove the user from the channel's user list and
 * notify the remaining users that the parting user has left the channel along
 * with an optional parting message provided by the parting user.
 */
extern int leave_channel(struct User *parting_user, char *channel_name,
                         char *parting_message);

/*
 * Iterates through the list of channels currently active on the server.
 * Searches the list for the provided target Channel struct to delete.
 * Once found, frees all memory allocated for the Channel struct and its
 * ChannelNode container struct and removes it from the list of currently active
 * channels.
 */
extern int delete_channel(struct Channel *target_channel);

#endif
