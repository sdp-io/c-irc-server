#include "command.h"
#include "channel.h"
#include "messages.h"
#include "network.h"
#include "structs.h"
#include "user.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static int handle_lusers_cmd(int sender_fd);

static int handle_motd_cmd(int sender_fd);

/*
 * Helper function that updates a user's registration status, server state, as
 * well as sending the appropriate numeric replies to the newly registered user.
 * Called when a user is found to be registered after setting their nickname or
 * username.
 */
static void register_new_user(struct User *user) {
  char reply_buf[BUF_SIZE];

  user_set_registered(user);

  char *nick = user->nick;
  char *username = user->user_name;
  char *host_name = user->host_name;

  format_reply(reply_buf, BUF_SIZE, RPL_WELCOME, SERVER_NAME, nick, nick,
               username, host_name);

  send_string(user->user_fd, reply_buf, strlen(reply_buf));

  // Send MOTD and LUSERS replies
  handle_lusers_cmd(user->user_fd);
  handle_motd_cmd(user->user_fd);
}

/*
 * Helper function called by the PRIVMSG+NOTICE handler. Called when the target
 * of the message is a channel. Attempts to relay the provided message to all of
 * the members of the target channel.
 */
static void handle_privmsg_channel(struct User *sender_user,
                                   char *recipient_channel, char *message,
                                   bool is_notice) {
  char reply_buf[BUF_SIZE];
  struct Channel *target_channel = get_channel(recipient_channel);

  // Send ERR_NOSUCHNICK numeric if command called is not a NOTICE
  if (target_channel == NULL) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHNICK, SERVER_NAME,
                   sender_user->nick, recipient_channel);

      send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    }
    return;
  }

  struct UserNode *sender_member =
      channel_get_member(target_channel, sender_user);

  // By default users cannot message channels in which they are not members
  if (sender_member == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_CANNOTSENDTOCHAN, SERVER_NAME,
                 sender_user->nick, recipient_channel);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return;
  }

  bool sender_has_voice = sender_user->is_oper || sender_member->channel_op ||
                          sender_member->channel_voice;

  // User does not have voice permission in moderated channel,
  // send numeric if command called is not a NOTICE
  if (target_channel->moderated_mode && !sender_has_voice) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_CANNOTSENDTOCHAN, SERVER_NAME,
                   sender_user->nick, recipient_channel);

      send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    }
    return;
  }

  // Otherwise prepare to send the message to the target
  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;
  char *fmt_string = is_notice ? FMT_NOTICE : FMT_PRIVMSG;

  // Format reply to match the command called by the sending user
  format_reply(reply_buf, BUF_SIZE, fmt_string, sender_user->nick,
               sender_username, sender_hostname, recipient_channel, message);

  channel_message_users(target_channel, reply_buf, sender_user->user_fd);
}

/*
 * Helper function called by the PRIVMSG+NOTICE handler. Called when the target
 * of the message is a user. Attempts to relay the provided message to the
 * target user.
 */
static void handle_privmsg_user(struct User *sender_user, char *target_nick,
                                char *message, bool is_notice) {
  char reply_buf[BUF_SIZE];
  struct User *target_user = get_user_by_nick(target_nick);

  // Send ERR_NOSUCHNICK numeric if command called is not a NOTICE
  if (target_user == NULL) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHNICK, SERVER_NAME,
                   sender_user->nick, target_nick);

      send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    }

    return;
  }

  // Reply with 301 RPL_AWAY numeric if user is away and cmd is not a NOTICE
  if (target_user != NULL && target_user->is_away) {
    if (!is_notice) {
      format_reply(reply_buf, BUF_SIZE, RPL_AWAY, SERVER_NAME,
                   sender_user->nick, target_user->nick, target_user->away_msg);

      send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    }

    return;
  }

  // Otherwise prepare to send the message to the target
  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;
  char *fmt_string = is_notice ? FMT_NOTICE : FMT_PRIVMSG;

  // Format reply to match the command called by the sending user
  format_reply(reply_buf, BUF_SIZE, fmt_string, sender_user->nick,
               sender_username, sender_hostname, target_nick, message);

  send_string(target_user->user_fd, reply_buf, strlen(reply_buf));
}

/*
 * Relays a message from the sender to the target recipient. Supports both
 * PRIVMSG and NOTICE commands. If the is_notice flag is set to true, suppresses
 * any replies that the sender may receive.
 */
static int handle_msg_cmd(int sender_fd, char *recipient_nick, char *message,
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

  // Message target is a channel
  if (recipient_nick[0] == '#') {
    handle_privmsg_channel(sender_user, recipient_nick, message, is_notice);
  } else { // Message target is a user
    handle_privmsg_user(sender_user, recipient_nick, message, is_notice);
  }

  return 0;
}

static int handle_nick_cmd(int sender_fd, char *nick_param) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char reply_buf[BUF_SIZE];

  // If nick contains invalid characters, do not need to verify its availability
  if (!is_valid_nick(nick_param)) {
    // Format and send ERR_ERRONEOUSNICKNAME numeric reply
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_ERRONEOUSNICKNAME, SERVER_NAME,
                 current_nick, nick_param);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  // If specified nick is taken, format and send ERR_NICKNAMEINUSE numeric
  if (get_user_by_nick(nick_param) != NULL) {
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_NICKNAMEINUSE, SERVER_NAME,
                 current_nick, nick_param);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  // Specified nick is available and valid
  set_user_nick(sender_user, nick_param);

  // Check if user is now newly registered
  bool has_nick = sender_user->has_nick;
  bool has_username = sender_user->has_username;
  bool is_registered = sender_user->is_registered;

  if (has_nick && has_username && !is_registered) {
    register_new_user(sender_user);
  }

  return 0;
}

static int handle_user_cmd(int sender_fd, char *user_param, char *mode_param,
                           char *realname_param) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char reply_buf[BUF_SIZE];

  // Verify integrity of the provided parameters
  if (user_param == NULL || mode_param == NULL || realname_param == NULL) {
    return -1;
  }

  // Sending user has a pre-existing username, format and send numeric reply
  if (sender_user->user_name != NULL || sender_user->real_name != NULL) {
    char *current_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

    format_reply(reply_buf, BUF_SIZE, ERR_ALREADYREGISTERED, SERVER_NAME,
                 current_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // Specified username and parameters are valid
  set_user_username(sender_user, user_param, realname_param);

  // Check if user is now newly registered
  bool has_nick = sender_user->has_nick;
  bool has_username = sender_user->has_username;
  bool is_registered = sender_user->is_registered;

  if (has_nick && has_username && !is_registered) {
    register_new_user(sender_user);
  }

  return 0;
}

/*
 * Receives a PING command from a sender along with a message parameter.
 * Formats the message into FMT_PING, then relays the PONG response back
 * to the sending user.
 */
static int handle_ping_cmd(int sender_fd, char *message) {
  char reply_buf[BUF_SIZE];

  if (message == NULL) {
    message = "";
  }

  format_reply(reply_buf, BUF_SIZE, FMT_PING, SERVER_NAME, SERVER_NAME,
               message);

  send_string(sender_fd, reply_buf, strlen(reply_buf));
  return 0;
}

static void handle_unknown_cmd(int sender_fd, char *command) {
  char reply_buf[BUF_SIZE];

  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = (sender_user->nick != NULL) ? sender_user->nick : "*";

  format_reply(reply_buf, BUF_SIZE, ERR_UNKNOWNCOMMAND, SERVER_NAME,
               sender_nick, command);

  send_string(sender_fd, reply_buf, strlen(reply_buf));
}

static int handle_motd_cmd(int sender_fd) {
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

/*
 * Helper function for the WHOIS command handler. Fills a buffer with the names
 * of the target's joined channels, formats it (RPL_WHOISCHANNELS) and sends the
 * numeric replies back to the sender.
 */
static void send_whois_channels(struct User *sender, struct User *target) {
  char reply_buf[BUF_SIZE];
  int channels_list_len = MAX_NICK_LEN * 6;
  char channels_list[channels_list_len];

  channels_list[0] = '\0'; // Prepare for strcat
  struct ChannelNode *current_channel = target->joined_channels;
  while (current_channel != NULL) {
    struct Channel *channel_info = current_channel->channel_info;
    struct UserNode *channel_user = channel_get_member(channel_info, target);
    char *channel_name = current_channel->channel_info->channel_name;
    char formatted_name[MAX_NICK_LEN + 3]; // Null-terminator and formatting

    // Add mode prefix to formatted channel name based on taret's modes
    if (channel_user->channel_op) {
      formatted_name[0] = '@';
      formatted_name[1] = '\0';
    } else if (channel_user->channel_voice) {
      formatted_name[0] = '+';
      formatted_name[1] = '\0';
    } else {
      formatted_name[0] = '\0';
    }

    strcat(formatted_name, channel_name);
    strcat(formatted_name, " "); // Space after first formatted name in list
    int total_len = strlen(formatted_name) + strlen(channels_list);

    // Send current channel names list before concatenation to prevent overflow
    if (total_len >= channels_list_len) {
      format_reply(reply_buf, BUF_SIZE, RPL_WHOISCHANNELS, SERVER_NAME,
                   sender->nick, target->nick, channels_list);

      send_string(sender->user_fd, reply_buf, strlen(reply_buf));

      channels_list[0] = '\0';
    }

    strcat(channels_list, formatted_name);

    current_channel = (struct ChannelNode *)current_channel->hh.next;
  }

  // Send any remaining contents of the channel names list
  if (channels_list[0] != '\0') {
    format_reply(reply_buf, BUF_SIZE, RPL_WHOISCHANNELS, SERVER_NAME,
                 sender->nick, target->nick, channels_list);

    send_string(sender->user_fd, reply_buf, strlen(reply_buf));
  }
}

static int handle_whois_cmd(int sender_fd, char *query_nick) {
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

  // Send RPL_WHOISCHANNELS
  if (query_user->joined_channels) {
    send_whois_channels(sender_user, query_user);
  }

  // Send RPL_WHOISSERVER (format with SERVER_NAME macro directly as we are
  // not currently a part of a network of servers)
  format_reply(reply_buf, BUF_SIZE, RPL_WHOISSERVER, SERVER_NAME, query_nick,
               SERVER_NAME, SERVER_INFO);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // Send RPL_AWAY
  if (query_user->is_away) {
    format_reply(reply_buf, BUF_SIZE, RPL_AWAY, SERVER_NAME, sender_user->nick,
                 query_nick, query_user->away_msg);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  // Send RPL_WHOISOPERATOR
  if (query_user->is_oper) {
    format_reply(reply_buf, BUF_SIZE, RPL_WHOISOPERATOR, SERVER_NAME,
                 sender_user->nick, query_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  // Send RPL_ENDOFWHOIS
  format_reply(reply_buf, BUF_SIZE, RPL_ENDOFWHOIS, SERVER_NAME, query_nick);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  return 0;
}

static int handle_lusers_cmd(int sender_fd) {
  int unknown_user_count = get_unknown_user_count();
  int registered_user_count = get_registered_user_count();

  // NOTE: Temporary variables for formatting of replies as services and
  // multiple servers are not currently supported
  int server_count = 0;
  int service_count = 0;

  int channel_count = channel_get_total();
  int operator_count = user_get_oper_count();

  char reply_buf[BUF_SIZE];

  // RPL_LUSERCLIENT reply
  format_reply(reply_buf, BUF_SIZE, RPL_LUSERCLIENT, SERVER_NAME,
               registered_user_count, service_count, server_count);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // RPL_LUSEROP reply
  format_reply(reply_buf, BUF_SIZE, RPL_LUSEROP, SERVER_NAME, operator_count);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // RPL_LUSERUNKNOWN reply
  format_reply(reply_buf, BUF_SIZE, RPL_LUSERUNKNOWN, SERVER_NAME,
               unknown_user_count);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // RPL_LUSERCHANNELS reply
  format_reply(reply_buf, BUF_SIZE, RPL_LUSERCHANNELS, SERVER_NAME,
               channel_count);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  // RPL_LUSERME reply
  format_reply(reply_buf, BUF_SIZE, RPL_LUSERME, SERVER_NAME,
               registered_user_count, server_count);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  return 0;
}

static int handle_names_cmd(int sender_fd, char *channel_param) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  if (channel_param) {
    struct Channel *target_channel = get_channel(channel_param);
    struct UserNode *channel_user = NULL;

    if (target_channel != NULL) {
      channel_user = target_channel->user_list;
    }

    int nick_list_len = MAX_NICK_LEN * 6; // 5 names with space for modes info
    char nick_list[nick_list_len];

    nick_list[0] = '\0'; // Prepare for strncat
    while (channel_user != NULL) {
      char *current_nick = channel_user->user_info->nick;
      char formatted_nick[MAX_NICK_LEN + 3]; // Null-terminator and formatting

      // Add mode prefix to formatted_nick before adding the nick
      if (channel_user->channel_op) {
        formatted_nick[0] = '@';
        formatted_nick[1] = '\0';
      } else if (channel_user->channel_voice) {
        formatted_nick[0] = '+';
        formatted_nick[1] = '\0';
      } else {
        formatted_nick[0] = '\0';
      }

      strcat(formatted_nick, current_nick);
      strcat(formatted_nick, " "); // Space after first formatted nick in list
      int total_len = strlen(formatted_nick) + strlen(nick_list);

      // Send current nick list before concatenation to prevent overflow
      if (total_len >= nick_list_len) {
        format_reply(reply_buf, BUF_SIZE, RPL_NAMREPLY, SERVER_NAME,
                     sender_nick, channel_param, nick_list);

        send_string(sender_fd, reply_buf, strlen(reply_buf));

        nick_list[0] = '\0';
      }

      strcat(nick_list, formatted_nick);

      channel_user = channel_user->next;
    }

    // Send any remaining contents of the nick list
    if (nick_list[0] != '\0') {
      format_reply(reply_buf, BUF_SIZE, RPL_NAMREPLY, SERVER_NAME, sender_nick,
                   channel_param, nick_list);

      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }
  }

  // Handle formatting for parameterless NAMES commands
  channel_param = channel_param != NULL ? channel_param : "*";

  format_reply(reply_buf, BUF_SIZE, RPL_ENDOFNAMES, SERVER_NAME, sender_nick,
               channel_param);

  send_string(sender_fd, reply_buf, strlen(reply_buf));

  return 0;
}

static int handle_join_cmd(int sender_fd, char *channel_name) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  if (channel_name == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NEEDMOREPARAMS, SERVER_NAME,
                 sender_nick, "JOIN");

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // IRC channel naming format validation, channels must be prefixed with '#'
  if (channel_name[0] != '#') {
    format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHCHANNEL, SERVER_NAME,
                 sender_nick, channel_name);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  struct Channel *joined_channel = NULL;
  int join_status = join_channel(sender_user, channel_name, &joined_channel);
  if (join_status == 1) {
    // User is already in the target channel, return silently
    return 0;
  } else if (join_status == -1) {
    printf("handle_join_cmd: error joining channel %s\n", channel_name);
    return -1;
  }

  user_add_channel(sender_user, joined_channel);

  // Relay the JOIN message to all users within the channel (IRC 2812, 3.1.2)
  format_reply(reply_buf, BUF_SIZE, FMT_JOIN, sender_nick,
               sender_user->user_name, sender_user->host_name, channel_name);

  channel_message_users(joined_channel, reply_buf, -1);

  // Relay the channel's topic to the joining user if there exists one
  if (joined_channel->topic != NULL) {
    format_reply(reply_buf, BUF_SIZE, RPL_TOPIC, SERVER_NAME, sender_nick,
                 channel_name, joined_channel->topic);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  // Relay the NAMES list for the channel to the sending user
  handle_names_cmd(sender_fd, channel_name);

  return 0;
}

static int handle_part_cmd(int sender_fd, char *channel_name,
                           char *parting_message) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  if (channel_name == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NEEDMOREPARAMS, SERVER_NAME,
                 sender_nick, "PART");

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  struct Channel *target_channel = get_channel(channel_name);
  int part_status =
      handle_part_channel_logic(sender_user, target_channel, parting_message);
  if (part_status == -1) {
    printf("handle_part_cmd: error parting from channel %s\n", channel_name);
    return -1;
  }

  return 0;
}

static int handle_topic_cmd(int sender_fd, char *channel_name,
                            char *topic_message) {
  char reply_buf[BUF_SIZE];

  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }
  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;

  // No provided channel parameter, send ERR_NEEDMOREPARAMS
  if (channel_name == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NEEDMOREPARAMS, SERVER_NAME,
                 sender_nick, "TOPIC");

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  struct Channel *target_channel = get_channel(channel_name);
  if (target_channel == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHCHANNEL, SERVER_NAME,
                 sender_nick, channel_name);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (!channel_has_user(target_channel, sender_user)) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTONCHANNEL, SERVER_NAME,
                 sender_nick, channel_name);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  struct UserNode *sender_member =
      channel_get_member(target_channel, sender_user);

  // Insufficient permissions to modify topic mode channel's topic
  if (target_channel->topic_mode && !sender_member->channel_op &&
      !sender_user->is_oper) {
    format_reply(reply_buf, BUF_SIZE, ERR_CHANOPRIVSNEEDED, SERVER_NAME,
                 sender_nick, channel_name);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // Empty channel topic string provided, remove channel's current topic
  if (topic_message != NULL && topic_message[0] == '\0') {
    channel_remove_topic(target_channel);

    // Format reply buffer with FMT_TOPIC and send to all users in channel
    format_reply(reply_buf, BUF_SIZE, FMT_TOPIC, sender_nick, sender_username,
                 sender_hostname, channel_name, topic_message);

    channel_message_users(target_channel, reply_buf, -1);

    return 0;
  }

  // Channel topic provided, change channel's current topic
  if (topic_message) {
    channel_set_topic(target_channel, topic_message);

    // Format reply buffer with FMT_TOPIC and send to all users in channel
    format_reply(reply_buf, BUF_SIZE, FMT_TOPIC, sender_nick, sender_username,
                 sender_hostname, channel_name, topic_message);

    channel_message_users(target_channel, reply_buf, -1);
    return 0;
  }

  // No channel topic provided, send channel's current topic to user
  char *channel_topic = target_channel->topic;
  if (channel_topic == NULL) {
    format_reply(reply_buf, BUF_SIZE, RPL_NOTOPIC, SERVER_NAME, sender_nick,
                 channel_name);
  } else {
    format_reply(reply_buf, BUF_SIZE, RPL_TOPIC, SERVER_NAME, sender_nick,
                 channel_name, channel_topic);
  }

  send_string(sender_fd, reply_buf, strlen(reply_buf));
  return 0;
}

static int handle_oper_cmd(int sender_fd, char *pass_param) {
  char reply_buf[BUF_SIZE];
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (pass_param == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NEEDMOREPARAMS, SERVER_NAME,
                 sender_nick, "OPER");

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (strcmp(pass_param, oper_password) == 0) {
    user_set_operator_status(sender_user, true);

    format_reply(reply_buf, BUF_SIZE, RPL_YOUREOP, SERVER_NAME, sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return 0;
  } else {
    format_reply(reply_buf, BUF_SIZE, ERR_PASSWDMISMATCH, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // Should theoretically be impossible to reach this point
  fprintf(stderr, "handle_oper_cmd: an unknown error has occurred\n");
  return -1;
}

static int handle_away_cmd(int sender_fd, char *away_msg) {
  char reply_buf[BUF_SIZE];
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  bool user_is_away = user_set_away_status(sender_user, away_msg);

  if (user_is_away) {
    format_reply(reply_buf, BUF_SIZE, RPL_NOWAWAY, SERVER_NAME, sender_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
  } else {
    format_reply(reply_buf, BUF_SIZE, RPL_UNAWAY, SERVER_NAME, sender_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
  }

  return 0;
}

static int handle_list_cmd(int sender_fd, char *channel_name) {
  char reply_buf[BUF_SIZE];
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // Handle LIST queries for a singular channel on the network
  if (channel_name != NULL) {
    struct Channel *target_channel = get_channel(channel_name);
    if (target_channel != NULL) {
      char *channel_name = target_channel->channel_name;
      int total_users = target_channel->total_users;
      char *topic = channel_get_topic(target_channel);

      format_reply(reply_buf, BUF_SIZE, RPL_LIST, SERVER_NAME, sender_nick,
                   channel_name, total_users, topic);

      send_string(sender_fd, reply_buf, strlen(reply_buf));
    }

    format_reply(reply_buf, BUF_SIZE, RPL_LISTEND, SERVER_NAME, sender_nick);
    send_string(sender_fd, reply_buf, strlen(reply_buf));
    return 0;
  }

  // Handle LIST queries for all channels on the network
  struct Channel *channels_iterator = channel_get_head();
  while (channels_iterator != NULL) {
    char *channel_name = channels_iterator->channel_name;
    int total_users = channels_iterator->total_users;
    char *topic = channel_get_topic(channels_iterator);

    format_reply(reply_buf, BUF_SIZE, RPL_LIST, SERVER_NAME, sender_nick,
                 channel_name, total_users, topic);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    channels_iterator = channel_get_next(channels_iterator);
  }

  format_reply(reply_buf, BUF_SIZE, RPL_LISTEND, SERVER_NAME, sender_nick);
  send_string(sender_fd, reply_buf, strlen(reply_buf));
  return 0;
}

static int handle_who_cmd(int sender_fd, char *mask_param) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  // Handle cases where the who command is being used on a channel
  if (mask_param != NULL && mask_param[0] == '#') {
    struct Channel *target_channel = get_channel(mask_param);
    struct UserNode *channel_users = NULL;
    if (target_channel != NULL) {
      channel_users = target_channel->user_list;
    }

    // Send RPL_WHOREPLY for all users within the specified channel
    while (channel_users != NULL) {
      struct User *current_user = channel_users->user_info;
      char *user_name = current_user->user_name;
      char *host_name = current_user->host_name;
      char *nickname = current_user->nick;
      char *real_name = current_user->real_name;
      bool is_chan_op = channel_users->channel_op;
      bool has_chan_voice = channel_users->channel_voice;
      char user_status[4];
      int hop_count = 0;

      if (current_user->is_away) {
        user_status[0] = 'G';
      } else {
        user_status[0] = 'H';
      }

      if (current_user->is_oper) {
        user_status[1] = '*';
      }

      // As channel operator takes precedent, check and set it first
      if (is_chan_op) {
        user_status[2] = '@';
        user_status[3] = '\0';
      } else if (has_chan_voice) {
        user_status[2] = '+';
        user_status[3] = '\0';
      } else {
        // No channel permissions so terminate only after away status
        user_status[2] = '\0';
      }

      format_reply(reply_buf, BUF_SIZE, RPL_WHOREPLY, SERVER_NAME, sender_nick,
                   mask_param, user_name, host_name, SERVER_NAME, nickname,
                   user_status, hop_count, real_name);

      send_string(sender_fd, reply_buf, strlen(reply_buf));

      channel_users = channel_users->next;
    }
  } else if (mask_param == NULL || mask_param[0] == '0' ||
             mask_param[0] == '*') {
    struct User *current_user = user_get_head();

    while (current_user != NULL) {
      if (!users_share_channel(sender_user, current_user)) {
        char *user_name = current_user->user_name;
        char *host_name = current_user->host_name;
        char *nickname = current_user->nick;
        char *real_name = current_user->real_name;
        char *mask_param = "*"; // No channel mask has been set
        char user_status[3];
        int hop_count = 0;

        if (current_user->is_away) {
          user_status[0] = 'G';
        } else {
          user_status[0] = 'H';
        }

        if (current_user->is_oper) {
          user_status[1] = '*';
          user_status[2] = '\0';
        } else {
          user_status[1] = '\0';
        }

        format_reply(reply_buf, BUF_SIZE, RPL_WHOREPLY, SERVER_NAME,
                     sender_nick, mask_param, user_name, host_name, SERVER_NAME,
                     nickname, user_status, hop_count, real_name);

        send_string(sender_fd, reply_buf, strlen(reply_buf));
      }

      current_user = user_get_next(current_user);
    }
  }

  mask_param = mask_param != NULL ? mask_param : "*";

  format_reply(reply_buf, BUF_SIZE, RPL_ENDOFWHO, SERVER_NAME, sender_nick,
               mask_param);

  send_string(sender_fd, reply_buf, strlen(reply_buf));
  return 0;
}

static int handle_quit_cmd(int sender_fd, char *quit_message) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick;
  char *sender_username = sender_user->user_name;
  char *sender_hostname = sender_user->host_name;
  char reply_buf[BUF_SIZE];

  // Set default quit message to sender nick (RFC 1459 4.1.6)
  if (!quit_message) {
    quit_message = sender_user->nick;
  }

  struct User *current_user = user_get_head();
  while (current_user != NULL) {
    bool users_share_chan = users_share_channel(sender_user, current_user);
    if (current_user != sender_user && users_share_chan) {
      // FMT and send QUIT message
      format_reply(reply_buf, BUF_SIZE, FMT_QUIT, sender_nick, sender_username,
                   sender_hostname, quit_message);

      send_string(current_user->user_fd, reply_buf, strlen(reply_buf));
    }

    current_user = user_get_next(current_user);
  }

  sender_user->is_dead = true;
  return 0;
}

/*
 * Helper function for handle_mode_cmd. Handles the setting of a Channel mode,
 * or a Channel User mode.
 */
static int handle_channel_mode(struct User *sender_user, char *channel_param,
                               char *mode_param, char *member_param) {
  struct Channel *target_channel = get_channel(channel_param);
  char reply_buf[BUF_SIZE];

  if (target_channel == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOSUCHCHANNEL, SERVER_NAME,
                 sender_user->nick, channel_param);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (target_channel && mode_param == NULL) {
    char mode_str[32]; // Arbitray length to hold supported modes
    mode_str[0] = '+';
    mode_str[1] = '\0';

    if (target_channel->moderated_mode) {
      strcat(mode_str, "m");
    }

    if (target_channel->topic_mode) {
      strcat(mode_str, "t");
    }

    format_reply(reply_buf, BUF_SIZE, RPL_CHANNELMODEIS, SERVER_NAME,
                 sender_user->nick, channel_param, mode_str);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return 0;
  }

  // Fail silently for single characters or "+"/"-" prefixes
  if (strlen(mode_param) < 2) {
    return -1;
  }

  // Retrieve sender's member info from the target channel parameter
  struct UserNode *sender_member =
      channel_get_member(target_channel, sender_user);

  if (sender_member == NULL) {
    // TODO: Send ERR_USERNOTINCHANNEL
    return -1;
  }

  if (!sender_member->channel_op && !sender_user->is_oper) {
    format_reply(reply_buf, BUF_SIZE, ERR_CHANOPRIVSNEEDED, SERVER_NAME,
                 sender_user->nick, channel_param);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  if (strcmp(mode_param, "+m") == 0) {
    if (!target_channel->moderated_mode) {
      channel_set_mode_moderated(target_channel, true);

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, "");

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  } else if (strcmp(mode_param, "-m") == 0) {
    channel_set_mode_moderated(target_channel, false);

    format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                 sender_user->user_name, sender_user->host_name, channel_param,
                 mode_param, "");

    channel_message_users(target_channel, reply_buf, -1);
    return 0;
  }

  if (strcmp(mode_param, "+t") == 0) {
    if (!target_channel->topic_mode) {
      channel_set_mode_topic(target_channel, true);

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, "");

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  } else if (strcmp(mode_param, "-t") == 0) {
    if (target_channel->topic_mode) {
      channel_set_mode_topic(target_channel, false);

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, "");

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  }

  // Mode parameter does not match any currently supported modes
  if (member_param == NULL) {
    char unknown_mode = mode_param[1];
    format_reply(reply_buf, BUF_SIZE, ERR_UNKNOWNMODE, SERVER_NAME,
                 sender_user->nick, unknown_mode, channel_param);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));

    return 0;
  }

  // Handle MODE commands on a channel member
  struct User *channel_member = get_user_by_nick(member_param);
  if (channel_member == NULL ||
      !channel_has_user(target_channel, channel_member)) {
    format_reply(reply_buf, BUF_SIZE, ERR_USERNOTINCHANNEL, SERVER_NAME,
                 sender_user->nick, member_param, channel_param);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  struct UserNode *channel_member_info =
      channel_get_member(target_channel, channel_member);

  if (strcmp(mode_param, "+o") == 0) {
    if (!channel_member_info->channel_op) {
      channel_member_info->channel_op = true;

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, channel_member->nick);

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  } else if (strcmp(mode_param, "-o") == 0) {
    if (channel_member_info->channel_op) {
      channel_member_info->channel_op = false;

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, channel_member->nick);

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  }

  if (strcmp(mode_param, "+v") == 0) {
    if (!channel_member_info->channel_voice) {
      channel_member_info->channel_voice = true;

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, channel_member->nick);

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  } else if (strcmp(mode_param, "-v") == 0) {
    if (channel_member_info->channel_voice) {
      channel_member_info->channel_voice = false;

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name,
                   channel_param, mode_param, channel_member->nick);

      channel_message_users(target_channel, reply_buf, -1);
    }
    return 0;
  }

  // Mode parameter does not match any currently supported modes
  char unknown_mode = mode_param[1];
  format_reply(reply_buf, BUF_SIZE, ERR_UNKNOWNMODE, SERVER_NAME,
               sender_user->nick, unknown_mode, channel_param);

  send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));

  return 0;
}

/*
 * Helper function for handle_mode_cmd. Handles the setting of a User mode only.
 */
static int handle_user_mode(struct User *sender_user, char *user_param,
                            char *mode_param) {
  char reply_buf[BUF_SIZE];

  if (strcasecmp(sender_user->nick, user_param) != 0) {
    format_reply(reply_buf, BUF_SIZE, ERR_USERSDONTMATCH, SERVER_NAME,
                 sender_user->nick);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return -1;
  }

  // Reply with user's current modes
  if (mode_param == NULL) {
    char mode_str[32]; // Arbitrary length to hold supported modes
    mode_str[0] = '+';
    mode_str[1] = '\0';

    if (sender_user->is_oper) {
      strcat(mode_str, "o");
    }

    if (sender_user->is_away) {
      strcat(mode_str, "a");
    }

    format_reply(reply_buf, BUF_SIZE, RPL_UMODEIS, SERVER_NAME,
                 sender_user->nick, mode_str);

    send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    return 0;
  }

  // Cannot set OPER mode via MODE cmd (RFC 2812, 3.1.5), Return silently
  if (strcmp(mode_param, "+o") == 0) {
    return 0;
  }

  if (strcmp(mode_param, "-o") == 0) {
    if (sender_user->is_oper) {
      user_set_operator_status(sender_user, false);

      format_reply(reply_buf, BUF_SIZE, FMT_MODE, sender_user->nick,
                   sender_user->user_name, sender_user->host_name, user_param,
                   mode_param, "");

      send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));
    }
    return 0;
  }

  // Cannot set AWAY mode via MODE cmd (RFC 2812, 3.1.5), Return silently
  if (strcmp(mode_param, "+a") == 0 || strcmp(mode_param, "-a") == 0) {
    return 0;
  }

  // Mode parameter does not match any currently supported modes
  char unknown_mode = mode_param[1];
  format_reply(reply_buf, BUF_SIZE, ERR_UNKNOWNMODE, SERVER_NAME,
               sender_user->nick, unknown_mode, sender_user->nick);

  send_string(sender_user->user_fd, reply_buf, strlen(reply_buf));

  return 0;
  return 0;
}

// NOTE: Currently only supports one MODE parameter
static int handle_mode_cmd(int sender_fd, char *target_param, char *mode_param,
                           char *member_param) {
  struct User *sender_user = get_user_by_fd(sender_fd);
  char *sender_nick = sender_user->nick != NULL ? sender_user->nick : "*";
  char reply_buf[BUF_SIZE];

  if (!sender_user->is_registered) {
    format_reply(reply_buf, BUF_SIZE, ERR_NOTREGISTERED, SERVER_NAME,
                 sender_nick);

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  if (target_param == NULL) {
    format_reply(reply_buf, BUF_SIZE, ERR_NEEDMOREPARAMS, SERVER_NAME,
                 sender_nick, "MODE");

    send_string(sender_fd, reply_buf, strlen(reply_buf));

    return -1;
  }

  if (target_param[0] == '#') {
    handle_channel_mode(sender_user, target_param, mode_param, member_param);
  }

  if (target_param[0] != '#') {
    handle_user_mode(sender_user, target_param, mode_param);
  }

  return 0;
}

void handle_user_msg(int sender_fd, char *buf) {
  // Split the buffer into individual lines to handle multiple commands
  // received in a single packet
  char *line_saveptr = NULL;
  char *user_cmd_line = strtok_r(buf, "\r\n", &line_saveptr);

  while (user_cmd_line != NULL) {
    // Copy the line so user's buffer is not overwritten and corrupted
    char line_copy[BUF_SIZE + 1];
    strncpy(line_copy, user_cmd_line, BUF_SIZE);
    line_copy[BUF_SIZE] = '\0';

    char *inner_saveptr = NULL;
    char *user_cmd = strtok_r(line_copy, " ", &inner_saveptr);

    // Check if only received carriage return from user
    if (user_cmd == NULL) {
      user_cmd_line = strtok_r(NULL, "\r\n", &line_saveptr);
      continue;
    }

    bool is_privmsg = (strcasecmp(user_cmd, "PRIVMSG") == 0);
    bool is_notice = (strcasecmp(user_cmd, "NOTICE") == 0);

    if ((strcasecmp(user_cmd, "NICK")) == 0) {
      char *nick_param = strtok_r(NULL, " ", &inner_saveptr);
      handle_nick_cmd(sender_fd, nick_param);
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

      handle_user_cmd(sender_fd, user_param, mode_param, realname_param);
    } else if (is_privmsg || is_notice) {
      char *recipient_param = strtok_r(NULL, " ", &inner_saveptr);
      char *message_param = strtok_r(NULL, "", &inner_saveptr);

      // Safely parse the colon from the beginning of the message param
      if (message_param != NULL && message_param[0] == ':') {
        message_param++;
      }

      handle_msg_cmd(sender_fd, recipient_param, message_param, is_notice);
    } else if ((strcasecmp(user_cmd, "JOIN")) == 0) {
      char *channel_param = strtok_r(NULL, " ", &inner_saveptr);
      handle_join_cmd(sender_fd, channel_param);
    } else if ((strcasecmp(user_cmd, "PART")) == 0) {
      char *channel_param = strtok_r(NULL, " ", &inner_saveptr);
      char *message_param = strtok_r(NULL, "", &inner_saveptr);

      if (message_param != NULL && message_param[0] == ':') {
        message_param++;
      }

      handle_part_cmd(sender_fd, channel_param, message_param);
    } else if ((strcasecmp(user_cmd, "TOPIC")) == 0) {
      char *channel_param = strtok_r(NULL, " ", &inner_saveptr);
      char *topic_param = strtok_r(NULL, "", &inner_saveptr);

      if (topic_param != NULL && topic_param[0] == ':') {
        topic_param++;
      }

      handle_topic_cmd(sender_fd, channel_param, topic_param);
    } else if ((strcasecmp(user_cmd, "WHOIS")) == 0) {
      char *nick_param = strtok_r(NULL, " ", &inner_saveptr);
      handle_whois_cmd(sender_fd, nick_param);
    } else if ((strcasecmp(user_cmd, "MOTD")) == 0) {
      handle_motd_cmd(sender_fd);
    } else if ((strcasecmp(user_cmd, "LUSERS")) == 0) {
      handle_lusers_cmd(sender_fd);
    } else if ((strcasecmp(user_cmd, "OPER")) == 0) {
      // Ignore required nick param is it is unused for our OPER implementation
      strtok_r(NULL, " ", &inner_saveptr);

      char *pass_param = strtok_r(NULL, " ", &inner_saveptr);

      handle_oper_cmd(sender_fd, pass_param);
    } else if ((strcasecmp(user_cmd, "AWAY")) == 0) {
      char *away_msg = strtok_r(NULL, "", &inner_saveptr);

      if (away_msg != NULL && away_msg[0] == ':') {
        away_msg++;
      }

      handle_away_cmd(sender_fd, away_msg);
    } else if ((strcasecmp(user_cmd, "LIST")) == 0) {
      char *channel_param = strtok_r(NULL, " ", &inner_saveptr);

      if (channel_param != NULL && channel_param[0] == ':') {
        channel_param++;
      }

      handle_list_cmd(sender_fd, channel_param);
    } else if ((strcasecmp(user_cmd, "WHO")) == 0) {
      char *mask_param = strtok_r(NULL, " ", &inner_saveptr);

      if (mask_param != NULL && mask_param[0] == ':') {
        mask_param++;
      }

      handle_who_cmd(sender_fd, mask_param);
    } else if ((strcasecmp(user_cmd, "NAMES")) == 0) {
      char *channel_param = strtok_r(NULL, " ", &inner_saveptr);

      if (channel_param != NULL && channel_param[0] == ':') {
        channel_param++;
      }

      handle_names_cmd(sender_fd, channel_param);
    } else if ((strcasecmp(user_cmd, "MODE")) == 0) {
      char *target_param = strtok_r(NULL, " ", &inner_saveptr);
      char *mode_param = strtok_r(NULL, " ", &inner_saveptr);
      char *member_param = strtok_r(NULL, " ", &inner_saveptr);

      handle_mode_cmd(sender_fd, target_param, mode_param, member_param);
    } else if ((strcasecmp(user_cmd, "PING")) == 0) {
      char *message_param = strtok_r(NULL, "", &inner_saveptr);
      handle_ping_cmd(sender_fd, message_param);
    } else if ((strcasecmp(user_cmd, "PONG")) == 0) {
      // Handle 'PONG' messages silently
    } else if ((strcasecmp(user_cmd, "QUIT")) == 0) {
      char *quit_message = strtok_r(NULL, "", &inner_saveptr);

      if (quit_message != NULL && quit_message[0] == ':') {
        quit_message++;
      }

      handle_quit_cmd(sender_fd, quit_message);
    } else {
      // Unknown command, send ERR_UNKNOWNCOMMAND
      handle_unknown_cmd(sender_fd, user_cmd);
    }

    user_cmd_line = strtok_r(NULL, "\r\n", &line_saveptr);
  }
}
