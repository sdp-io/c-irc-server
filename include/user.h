#ifndef USER_H
#define USER_H

extern int add_to_users(int user_fd, char *user_host);

extern void del_from_users(int user_fd);

extern void handle_user_msg(int sender_fd, char *buf);

extern int set_user_nick(int sender_fd, char *sender_nick);

extern void set_user_username(int sender_fd, char *user_param, char *mode_param,
                              char *realname_param);

extern struct User *get_user(int query_fd);

#endif
