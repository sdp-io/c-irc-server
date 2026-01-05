#include "messages.h"
#include "network.h"
#include "structs.h"
#include "user.h"
#include "utils.h"
#include <string.h>

int handle_privmsg_cmd(int sender_fd, char *recipient_nick, char *message) {
  if (recipient_nick == NULL) {
    // TODO: Send ERR_NORECIPIENT numeric
    return -1;
  }

  if (message == NULL) {
    // TODO: Send NO_TEXTTOSEND numeric
    return -1;
  }

  struct User *sender_user = get_user_by_fd(sender_fd);

  if (!sender_user->is_registered) {
    // TODO: Send ERR_NOTREGISTERED numeric
    return -1;
  }

  struct User *recipient_user = get_user_by_nick(recipient_nick);

  if (recipient_user == NULL) {
    // TODO: Send ERR_NOSUCHNICK numeric
    return -1;
  }

  char reply_buf[BUF_SIZE];
  char *sender_nickname = sender_user->nick;
  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;

  format_reply(reply_buf, BUF_SIZE, FMT_PRIVMSG, sender_nickname,
               sender_username, sender_hostname, recipient_nick, message);
  send_string(recipient_user->user_fd, reply_buf, strlen(reply_buf));

  return 0;
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
    char *realname_param = strtok(NULL, "\r\n");

    // Safely parse the colon from the beginning of the realname param
    if (realname_param != NULL && realname_param[0] == ':') {
      realname_param++;
    }

    set_user_username(sender_fd, user_param, mode_param, realname_param);
  }

  if ((strcasecmp(user_cmd, "PRIVMSG")) == 0) {
    char *recipient_param = strtok(NULL, " \r\n");
    char *message_param = strtok(NULL, "\r\n");

    // Safely parse the colon from the beginning of the message param
    if (message_param != NULL && message_param[0] == ':') {
      message_param++;
    }

    handle_privmsg_cmd(sender_fd, recipient_param, message_param);
  }
}
