#define _GNU_SOURCE
#include "fmt.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** @brief Number format spec */
struct number_spec
{
	/** @brief Fill character (codepoint) */
	char fill[5];
	/**
	 * @brief Alignment character:
	 *  - `<`: Left (default)
	 *  - `>`: Right
	 *  - `^`: Center
	 */
	char align;
	/**
	 * @brief Sign specifier
	 *  - `-`: Display sign for negatives only (default)
	 *  - `+`: Always display sign
	 *  - ` `: Display sign for negative and leave a blank for positives
	 */
	char sign;
	/** @brief Alternate mode */
	int alternate;
	/** @brief Flag to force padding numbers with `0`'s */
	int zero;
	/** @brief Minimum field width */
	size_t width;
	/** @brief Field precision */
	size_t precision;
	/** @brief Display type */
	char type;
	/** @brief Leftover, whatever is left after parsing the previous fields */
	const char* left;
};

/**
 * @brief Compute the length of a unicode codepoint
 *
 * @param str Codepoint start byte
 * @param len Maximum length to search @p str for
 *
 * @return The length of the UTF-8 codepoint starting at `*str`
 */
static inline size_t
utf8_len(const char* str, size_t len)
{
	if (!len)
		return 0;
	size_t s;
	if ((str[0] & 0x80) == 0)
		s = 1;
	else if ((str[0] & 0xE0) == 0xC0)
		s = 2;
	else if ((str[0] & 0xF0) == 0xE0)
		s = 3;
	else if ((str[0] & 0xF8) == 0xF0)
		s = 4;
	else if ((str[0] & 0xFC) == 0xF8)
		s = 5;
	else if ((str[0] & 0xFE) == 0xFC)
		s = 6;
	else
		return 0;
	if (s > len)
		return 0;
	if (strnlen(str, s) < s)
		return 0;
	return s;
}

/**
 * @brief Parse a numeric value from a format specifier
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
static inline size_t
parse_number(const char* fmt_spec, size_t* i, const struct fmt_env env)
{
	size_t size = 0;
	/* Reference */
	if (fmt_spec[*i] == '{') {
		++*i;
		assert(isdigit(fmt_spec[*i]) && "Expected digit after `{'");
		while (isdigit(fmt_spec[*i])) {
			size = size * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
			assert(size < env.size &&
			       "Cannot reference element past the number of formatting arguments");
		}
		size = *(const size_t*)env.args[size].data;
		assert(size <= 16384 && "Size cannot exceed 16384");
		assert(fmt_spec[*i] == '}' && "Expected `}' after number");
		++*i;
	}
	/* Literal */
	else if (isdigit(fmt_spec[*i])) {
		while (isdigit(fmt_spec[*i])) {
			size = size * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
			assert(size <= 16384 && "Size cannot exceed 16384");
		}
	}
	return size;
}

/**
 * @brief Parse the format specifier for numbers
 *
 * @param fmt_spec Format specifier
 * @param env Format environment
 *
 * @return Parsed @ref number_spec for @p fmt_spec
 */
static inline struct number_spec
parse_number_spec(const char* fmt_spec, const struct fmt_env env)
{
	struct number_spec spec = {
		.fill = { ' ', 0, 0, 0, 0 }, /* Space */
		.align = '<',          /* Left */
		.sign = '-',           /* Negative only */
		.alternate = 0,
		.zero = 0,
		.width = 0,
		.precision = 0,
		.type = 0,
		.left = fmt_spec,
	};
	if (*fmt_spec == '}')
		return spec;

	assert(strchr(fmt_spec, '}') != NULL &&
	       "Expected `}'"); // TODO: Redundant, should be asserted before calling this

	size_t i = 0;
	/* Parse align */
	const size_t len = utf8_len(fmt_spec, 5); /* Get width of the alignment character codepoint */
	assert(len <= 5);
	if (strchr("<>^", fmt_spec[len]))         /* Custom character */
	{
		spec.align = fmt_spec[len];
		strncpy((char*)spec.fill, fmt_spec, len);
		i = len + 1;
	} else if (strchr("<>^", fmt_spec[i])) /* Default character */
	{
		spec.align = fmt_spec[i++];
	}

	/* Parse sign */
	if (strchr("-+ ", fmt_spec[i]))
		spec.sign = fmt_spec[i++];

	/* Parse alternate mode */
	if (fmt_spec[i] == '#') {
		spec.alternate = 1;
		++i;
	}

	/* Parse zero */
	if (fmt_spec[i] == '0') {
		spec.zero = 1;
		++i;
	}

	/* Parse width */
	spec.width = parse_number(fmt_spec, &i, env);

	/* Parse precision */
	if (fmt_spec[i] == '.') {
		++i;
		spec.precision = parse_number(fmt_spec, &i, env);
	}

	/* Parse type */
	if (fmt_spec[i] != '}')
		spec.type = fmt_spec[i++];

	spec.left = fmt_spec + i;
	return spec;
}

static inline void
write_aligned(struct format_output *output, const struct number_spec *spec, const char *buf, size_t len)
{
	/* Padding */
	size_t right = 0, left = 0;
	switch (spec->align)
	{
		/* Center */
		case '^':
			right = spec->width / 2;
			left = spec->width - right;
			break;
		/* Left */
		case '<':
			left = spec->width;
			break;
		/* Right */
		case '>':
			right = spec->width;
			break;
		default:
			__builtin_unreachable();
	}

	/* Write */
	const size_t fill_len = strnlen(spec->fill, 5);
	if (right > len) /* Right */
	{
		for (size_t i = 0; i < right - len; ++i)
			format_output_write(output, spec->fill, fill_len);
	}
	format_output_write(output, buf, len);
	if (left > len) /* Left */
	{
		for (size_t i = 0; i < left - len; ++i)
			format_output_write(output, spec->fill, fill_len);
	}
}

void
format_fmt_long(struct format_output* output,
                const char* fmt_spec,
                const struct fmt_env env,
                size_t idx)
{
	struct number_spec spec = parse_number_spec(fmt_spec, env);

	assert(spec.zero == 0);
	assert(spec.precision == 0 && "Unsupported precision");
	assert(strchr("xXbB", spec.type) != NULL && "Invalid display type");
	assert(spec.left[0] == '}' && "Leftover content in format specifier");

	const long val = *(const long*)env.args[idx].data;
	char buf[sizeof(long) * 8 + 16];

	/* Get base */
	const int base = (tolower(spec.type) == 'x') ? 16 : (tolower(spec.type) == 'b') ? 2 : 10;

	/* Compute length in base */
	size_t len = 0;
	for (long x = val; x; x /= base)
		++len;
	if (val == 0)
		len = 1;

	/* Sign */
	size_t start = 0;
	if (spec.sign != '-') {
		++len;
		buf[start++] = spec.sign == '+' ? '+' : ' ';
	}

	/* Prefix */
	if (spec.alternate && spec.type != '\0') {
		buf[start++] = '0';
		buf[start++] = spec.type;
		len += 2;
	}

	/* Value */
	if (val == 0)
		buf[start] = '0';
	else {
		long x = val;
		for (size_t i = 0; x; ++i) {
			const int d = (int)(x % base);
			switch (spec.type) {
				case 'x':
					buf[len - i - 1] = "01234567489abcdef"[d];
					break;
				case 'X':
					buf[len - i - 1] = "01234567489ABCDEF"[d];
					break;
				default:
					buf[len - i - 1] = (char)('0' + d);
					break;
			}
		}
	}

	write_aligned(output, &spec, buf, len);
}
