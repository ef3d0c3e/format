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

#define FORMAT_HEX "0123456789abcdef"
#define FORMAT_HEX_CAPITAL "0123456789ABCDEF"

/** @brief Represent a placeholder character */
struct spec_placeholder {
	/**
	 * @brief Placeholder type
	 *  - `0`: Codepoint
	 *  - `1`: String
	 */
	int type;
	union {
		char codepoint[5];
		const char* str;
	};
	/** @brief Number of bytes in the string (strlen) */
	size_t len;
	/** @brief Number of codepoints in the string */
	size_t width;
};

int
write_placeholder(struct format_output* output,
                  const struct spec_placeholder* placeholder,
                  size_t max_width,
				  int reverse);

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
 * Or a reference to an argument in @ref format_env: `{1}`, `{0}`, ...
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
parse_size(const char* fmt_spec, size_t* i, const struct format_env* env);

size_t
parse_number(const char* fmt_spec, size_t* i, const struct format_env* env);

struct spec_placeholder
parse_placeholder(const char *fmt_spec, size_t *i, const struct format_env *env);

void
parse_alignment(const char *fmt_spec, size_t *i, const struct format_env *env, char *alignment, struct spec_placeholder* fill, const char *default_placeholder);

int
format_fmt_collection(struct format_output* output,
               const char* fmt_spec,
               const struct format_env* env,
               size_t idx);

#endif // LIBFORMAT_FMT_H
