#include "utils.h"
#include "structs.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
 * Given a character, verifies that the character is permitted to
 * act as the first character of a user nickname (See RFC 2812 2.3.1)
 */
static bool is_valid_first_char(char letter) {
  char *special_chars = "[]\\`_^{|}";

  if (strchr(special_chars, letter) != NULL) {
    return true;
  }

  if (isalpha(letter)) {
    return true;
  }

  return false;
}

/*
 * Ensures that a proposed nickname is valid according to RFC 2818 2.3.1
 */
bool is_valid_nick(char *nick) {
  if ((nick == NULL) || !is_valid_first_char(nick[0])) {
    return false;
  }

  if (strlen(nick) > MAX_NICK_LEN) {
    return false;
  }

  return true;
}

/*
 * Variadic function which formats a provided buffer based on
 * the provided numeric reply or message format.
 */
void format_reply(char *buf, int bufsize, char *format, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, format);

  vsnprintf(buf, bufsize, format, arg_ptr);

  va_end(arg_ptr);
}
