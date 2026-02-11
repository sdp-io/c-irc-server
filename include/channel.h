#ifndef CHANNEL_H
#define CHANNEL_H

#include "structs.h"

// TODO: Add documentation
extern int join_channel(struct User *joining_user, char *channel_name);

// TODO: Add documentation
extern int leave_channel(struct User *parting_user, char *channel_name,
                         char *parting_message);

// TODO: Add documentation
extern int delete_channel(struct Channel *target_channel);

#endif
