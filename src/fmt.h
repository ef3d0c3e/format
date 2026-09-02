#ifndef LIBFORMAT_FMT_H
#define LIBFORMAT_FMT_H

#define _GNU_SOURCE

#include "buffer.h"

#include <unistd.h>
#include <errno.h>

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Compute the length of a unicode codepoint
 *
 * @param str Codepoint start byte
 * @param len Maximum length to search @p str for
 *
 * @return The length of the UTF-8 codepoint starting at `*str`
 * @return `0` if len is 0 or the sequence `str[0]..str[len]` does not start with a valid codepoint
 */
size_t
utf8_len(const char* str, size_t len);

/**
 * @brief Parse a numeric value associated to a size from a format specifier
 *
 * A numeric value is either an integer literal: `0`, `5`, `123456`, ...
 * Or a reference to an argument in @ref fmt_env: `{1}`, `{0}`, ...
 *
 * This function may not parse a size greater than 16384 for safety reasons.
 * It will assert to make sure this doesn't happen.
 *
 * @param fmt_spec Format specifier
 * @param i Index to start parsing at (will advance)
 * @param env Formatting environment
 *
 * @return The parsed numeric value
 */
size_t
parse_size(const char* fmt_spec, size_t* i, const struct fmt_env* env);

#endif // LIBFORMAT_FMT_H
