/* format -- C formatting library
 * Copyright (C) 2026 ef3d0c3e
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#include "fmt.h"

/**
 * @file fmt_char.c
 * @brief Char formatting
 */

struct char_spec
{
	/** @brief Fill string */
	struct format_spec_placeholder fill;
	/**
	 * @brief Alignment character:
	 *  - `<`: Left (default)
	 *  - `>`: Right
	 *  - `^`: Center
	 */
	char align;
	/** @brief Minimum field width */
	size_t width;
	/** @brief Toggle quoted mode */
	int quoted;
	/** @brief Left quote */
	struct format_spec_placeholder left_quote;
	/** @brief Right quote */
	struct format_spec_placeholder right_quote;
	/**
	 * @brief Display type
	 *  - 'c' Display characters as is (default)
	 *  - '?' Display non printable using common escape sequences or as `\xXX`
	 *  - 'x' Display non printables as `0xXX`
	 */
	char type;
	/** @brief Leftover, whatever is left after parsing the previous fields */
	const char* left;
};

static inline struct char_spec
parse_char_spec(const char* fmt_spec, const struct format_env* env)
{
	struct char_spec spec = {
		.align = '<', /* Left-aligned by default */
		.width = 0,
		.quoted = 0,
		.type = 'c',
		.left = fmt_spec,
	};

	if (*fmt_spec == '}')
		return spec;

	size_t i = 0;
	/* Parse align */
	format_parse_alignment(fmt_spec, &i, env, &spec.align, &spec.fill, " ");

	/* Parse width */
	spec.width = format_parse_size(fmt_spec, &i, env);

	/* Parse quotes */
	if (fmt_spec[i] == '#') {
		++i;
		spec.quoted = 1;
		spec.left_quote = format_parse_placeholder(fmt_spec, &i, env);
		spec.right_quote = format_parse_placeholder(fmt_spec, &i, env);
	}

	/* Parse type */
	if (fmt_spec[i] != '}') {
		spec.type = fmt_spec[i++];
		assert(strchr("c?x", spec.type) && "Invalid display type");
	}

	spec.left = fmt_spec + i;
	return spec;
}

int
format_fmt_char(struct format_output* output,
                const char* fmt_spec,
                const struct format_env* env,
                size_t idx)
{
	struct char_spec spec = parse_char_spec(fmt_spec, env);
	assert(spec.left[0] == '}' && "Leftover content in format specifier");

	const char val = (char)env->args[idx].data;

	/* Compute display width */
	size_t width = 0;
	if (spec.quoted)
		width += spec.left_quote.width + spec.right_quote.width;
	if (isprint(val) || val == '\t')
		width += 1;
	else if (val != 0 && strchr("\a\b\n\v\f\r", val) && spec.type == '?')
		width += 2;
	else
		width += spec.type == 'c' ? 1 : 4 /* \xXX or 0xXX */;

	/* Compute alignment */
	size_t left = 0, right = 0;
	switch (spec.align) {
		case '^':
			left = (spec.width > width ? spec.width - width : 0);
			right = left / 2;
			left -= right;
			break;
		case '<':
			right = spec.width > width ? spec.width - width : 0;
			break;
		case '>':
			left = spec.width > width ? spec.width - width : 0;
			break;
		default:
			format_unreachable();
	}

	/* Left spacing */
	if (format_write_placeholder(output, &spec.fill, left, 0))
		return -1;
	/* Left quote */
	if (spec.quoted) {
		if (format_write_placeholder(output, &spec.left_quote, (size_t)-1, 0))
			return -1;
	}

	/* Content */
	char buf[16];
	if (isprint(val) || val == '\t') {
		if (format_output_write(output, &val, 1))
			return -1;
	} else if (val != 0 && strchr("\a\b\n\v\f\r", val) && spec.type == '?') {
		buf[0] = '\\';
		switch (val) {
			case '\a':
				buf[1] = 'a';
				break;
			case '\b':
				buf[1] = 'b';
				break;
			case '\n':
				buf[1] = 'n';
				break;
			case '\v':
				buf[1] = 'v';
				break;
			case '\f':
				buf[1] = 'f';
				break;
			case '\r':
				buf[1] = 'r';
				break;
			default:
				format_unreachable();
		}
		if (format_output_write(output, buf, 2))
			return -1;
	} else {
		switch (spec.type) {
			case 'c':
				if (format_output_write(output, &val, 1))
					return -1;
				break;
			case '?':
				buf[0] = '\\';
				buf[1] = 'x';
				buf[2] = FORMAT_HEX[(unsigned char)val / 16];
				buf[3] = FORMAT_HEX[(unsigned char)val % 16];
				if (format_output_write(output, buf, 4))
					return -1;
				break;
			case 'x':
				buf[0] = '0';
				buf[1] = 'x';
				buf[2] = FORMAT_HEX[(unsigned char)val / 16];
				buf[3] = FORMAT_HEX[(unsigned char)val % 16];
				if (format_output_write(output, buf, 4))
					return -1;
				break;
			default:
				format_unreachable();
		}
	}

	/* Right quote */
	if (spec.quoted) {
		if (format_write_placeholder(output, &spec.right_quote, (size_t)-1, 0))
			return -1;
	}
	/* Right spacing */
	if (format_write_placeholder(output, &spec.fill, right, 1))
		return -1;

	return 0;
}
