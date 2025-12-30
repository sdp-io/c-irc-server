#include "structs.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/*
 * Given a character, verifies that the character is permitted to
 * act as the first character of a user nickname (See RFC 2812 2.3.1)
 */
int is_valid_first_char(char letter) {
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
 * Given a nickname string, confirms that the string is not null, contains
 * a valid first character, and does not exceed the MAX_NICK_LEN.
 */
int is_valid_nick(char *nick) {
  // TODO: Replace bools with RFC compliant reply codes
  if ((nick == NULL) || !is_valid_first_char(nick[0])) {
    return false;
  }

  if (strlen(nick) > MAX_NICK_LEN) {
    return false;
  }

  return true;
}
