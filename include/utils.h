#ifndef UTILS_H
#define UTILS_H

/*
 * Given a nickname string, confirms that the string is not null, contains
 * a valid first character, and does not exceed the MAX_NICK_LEN.
 */
extern int is_valid_nick(char *nick);

/*
 * Given a global numeric reply macro, formats the
 * string based on its matching corresponding args and write it to the provided
 * buffer param.
 */
extern void format_reply(char *buf, int bufsize, char *format, ...);

#endif
