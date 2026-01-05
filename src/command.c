#include "user.h"
#include <string.h>

int handle_privmsg_cmd(char *recipient, char *message) {
  if (recipient == NULL) {
    // TODO: Send ERR_NORECIPIENT numeric
    return;
  }

  if (message == NULL) {
    // TODO: Send NO_TEXTTOSEND numeric
    return;
  }
}

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

  if ((strcasecmp(user_cmd, "USER")) == 0) {
    char *user_param = strtok(NULL, " \r\n");
    char *mode_param = strtok(NULL, " \r\n");
    strtok(NULL, " \r\n"); // Skip the unused 'server' param (RFC 1459)
    char *realname_param = strtok(NULL, ":\r\n");
    set_user_username(sender_fd, user_param, mode_param, realname_param);
  }

  if ((strcasecmp(user_cmd, "PRIVMSG")) == 0) {
    char *recipient_param = strtok(NULL, " \r\n");
    char *message_param = strtok(NULL, ":\r\n");
    handle_privmsg_cmd(recipient_param, message_param);
  }
}
