#include "messages.h"
#include "network.h"
#include "structs.h"
#include "user.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int handle_msg_cmd(int sender_fd, char *recipient_nick, char *message,
                   bool is_notice) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nickname = (sender_user->nick != NULL) ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  // Send ERR_NORECIPIENT numeric is command called is not a NOTICE
  if (recipient_nick == NULL) {
    if (!is_notice) {
      char *cmd_name = "PRIVMSG";
      format_reply(reply_buf, BUF_SIZE, ERR_NORECIPIENT, SERVER_NAME,
                   sender_nickname, cmd_name);
      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }
    return -1;
  }

  // Send ERR_NOTEXTTOSEND numeric if command called is not a NOTICE
  if (message == NULL) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_NOTEXTTOSEND, SERVER_NAME,
                   sender_nickname);
      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }
    return -1;
  }

  // Send ERR_NOTREGISTERED numeric if command called is not a NOTICE
  if (!sender_user->is_registered) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                   sender_nickname);
      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }
    return -1;
  }

  struct User *recipient_user = get_user_by_nick(recipient_nick);

  // Send ERR_NOSUCHNICK numeric if command called is not a NOTICE
  if (recipient_user == NULL) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHNICK, SERVER_NAME,
                   sender_nickname, recipient_nick);
      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }
    return -1;
  }

  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;

  // Format reply to match the command called by the sending user
  if (!is_notice) {
    format_reply(reply_buf, BUF_SIZE, FMT_PRIVMSG, sender_nickname,
                 sender_username, sender_hostname, recipient_nick, message);
  } else {
    format_reply(reply_buf, BUF_SIZE, FMT_NOTICE, sender_nickname,
                 sender_username, sender_hostname, recipient_nick, message);
  }

  send_string(recipient_user->user_fd, reply_buf, strlen(reply_buf));
  return 0;
}

int handle_ping_cmd(int sender_fd, char *message) {
  char reply_buf[BUF_SIZE];

  // Send ERR_NOORIGIN
  if (message == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOORIGIN, SERVER_NAME);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  format_reply(reply_buf, BUF_SIZE, FMT_PING, SERVER_NAME, SERVER_NAME,
               message);
  send_string(sender_fd, reply_buf, strlen(reply_buf));
  return 0;
}

void handle_unknown_cmd(int sender_fd, char *command) {
  char reply_buf[BUF_SIZE];
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

  format_reply(reply_buf, BUF_SIZE, ERR_UNKNOWNCOMMAND, SERVER_NAME,
               sender_nick, command);
  send_string(sender_fd, reply_buf, strlen(reply_buf));
}

int handle_motd_cmd(int sender_fd) {
  FILE *motd_file;

  if ((motd_file = fopen("motd.txt", "r")) == NULL) {
    fprintf(stderr, "handle_motd_cmd: error opening motd.txt\n");
    return -1;
  }

  char motd_buf[MOTD_LINE_SIZE];
  char reply_buf[BUF_SIZE];

  format_reply(reply_buf, sizeof(reply_buf), RPL_MOTDSTART, SERVER_NAME,
               SERVER_NAME);
  send_string(sender_fd, reply_buf, strlen(reply_buf));

  while (fgets(motd_buf, sizeof(motd_buf), motd_file)) {
    motd_buf[strcspn(motd_buf, "\r\n")] = '\0'; // Trim carriage return
    format_reply(reply_buf, sizeof(reply_buf), RPL_MOTD, SERVER_NAME, motd_buf);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  format_reply(reply_buf, sizeof(reply_buf), RPL_ENDOFMOTD, SERVER_NAME);
  send_string(sender_fd, reply_buf, strlen(reply_buf));

  fclose(motd_file);
  return 0;
}

int handle_whois_cmd(int sender_fd, char *query_nick) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char reply_buf[BUF_SIZE];

  if (!sender_user->is_registered) {
    char *sender_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (query_nick == NULL) {
    // Handle targetless WHOIS calls silently
    return -1;
  }

  struct User *query_user = get_user_by_nick(query_nick);

  if (!query_user || !query_user->is_registered) {
    // Receive nickname of sender to format ERR_NOSUCHNICK reply
    char *sender_nick = sender_user->nick;
    format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHNICK, SERVER_NAME, sender_nick,
                 query_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  char *query_username = query_user->user_name;
  char *query_hostname = query_user->host_name;
  char *query_realname = query_user->real_name;

  // Send RPL_WHOISUSER
  format_reply(reply_buf, BUF_SIZE, RPL_WHOISUSER, SERVER_NAME, query_nick,
               query_username, query_hostname, query_realname);
  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // Send RPL_WHOISSERVER (format with SERVER_NAME macro directly as we are not
  // currently a part of a network of servers)
  format_reply(reply_buf, BUF_SIZE, RPL_WHOISSERVER, SERVER_NAME, query_nick,
               SERVER_NAME, SERVER_INFO);
  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // Send RPL_ENDOFWHOIS
  format_reply(reply_buf, BUF_SIZE, RPL_ENDOFWHOIS, SERVER_NAME, query_nick);
  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // TODO: Add further server replies once channels are implemented
  return 0;
}

void handle_user_msg(int sender_fd, char *buf) {
  // Split the buffer into individual lines to handle multiple commands received
  // in a single packet
  char *line_saveptr = NULL;
  char *inner_saveptr = NULL;
  char *user_cmd_line = strtok_r(buf, "\r\n", &line_saveptr);

  while (user_cmd_line != NULL) {
    char *user_cmd = strtok_r(user_cmd_line, " ", &inner_saveptr);

    // Check if only received carriage return from user
    if (user_cmd == NULL) {
      user_cmd_line = strtok_r(NULL, "\r\n", &line_saveptr);
      continue;
    }

    bool is_privmsg = (strcasecmp(user_cmd, "PRIVMSG") == 0);
    bool is_notice = (strcasecmp(user_cmd, "NOTICE") == 0);

    if ((strcasecmp(user_cmd, "NICK")) == 0) {
      char *nick_param = strtok_r(NULL, " ", &inner_saveptr);
      set_user_nick(sender_fd, nick_param);
    } else if ((strcasecmp(user_cmd, "USER")) == 0) {
      char *user_param = strtok_r(NULL, " ", &inner_saveptr);
      char *mode_param = strtok_r(NULL, " ", &inner_saveptr);

      // Skip the unused 'server' param (RFC 1459)
      strtok_r(NULL, " ", &inner_saveptr);

      char *realname_param = strtok_r(NULL, "", &inner_saveptr);

      // Safely parse the colon from the beginning of the realname param
      if (realname_param != NULL && realname_param[0] == ':') {
        realname_param++;
      }

      set_user_username(sender_fd, user_param, mode_param, realname_param);
    } else if (is_privmsg || is_notice) {
      char *recipient_param = strtok_r(NULL, " ", &inner_saveptr);
      char *message_param = strtok_r(NULL, "", &inner_saveptr);

      // Safely parse the colon from the beginning of the message param
      if (message_param != NULL && message_param[0] == ':') {
        message_param++;
      }

      handle_msg_cmd(sender_fd, recipient_param, message_param, is_notice);
    } else if ((strcasecmp(user_cmd, "WHOIS")) == 0) {
      char *nick_param = strtok_r(NULL, " ", &inner_saveptr);
      handle_whois_cmd(sender_fd, nick_param);
    } else if ((strcasecmp(user_cmd, "MOTD")) == 0) {
      handle_motd_cmd(sender_fd);
    } else if ((strcasecmp(user_cmd, "PING")) == 0) {
      char *message_param = strtok_r(NULL, "", &inner_saveptr);
      handle_ping_cmd(sender_fd, message_param);
    } else if ((strcasecmp(user_cmd, "PONG")) == 0) {
      // Handle 'PONG' messages silently
    } else {
      // Unknown command, send ERR_UNKNOWNCOMMAND
      handle_unknown_cmd(sender_fd, user_cmd);
    }

    user_cmd_line = strtok_r(NULL, "\r\n", &line_saveptr);
  }
}
