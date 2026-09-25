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

size_t
format_utf8_len(const char* str, size_t len)
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

size_t
utf8_len_str(const char* str, size_t len)
{
	size_t u8len = 0;
	for (size_t i = 0; i < len;) {
		const size_t cp = format_utf8_len(str + i, len - i);
		i += cp;
		++u8len;
	}
	return u8len;
}

size_t
format_parse_size(const char* fmt_spec, size_t* i, const struct format_env* env)
{
	size_t size = 0;
	/* Reference */
	if (fmt_spec[*i] == '{') {
		++*i;
		assert(isdigit(fmt_spec[*i]) && "Expected digit after `{'");
		while (isdigit(fmt_spec[*i])) {
			size = size * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
			assert(size < env->size &&
			       "Cannot reference element past the number of formatting arguments");
		}
		assert(fmt_spec[*i] == '}' && "Expected `}' after number");
		++*i;
		size = (size_t)env->args[size].data;
		assert(size <= 16384 && "Size cannot exceed 16384");
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

size_t
format_parse_number(const char* fmt_spec, size_t* i, const struct format_env* env)
{
	size_t number = 0;
	/* Reference */
	if (fmt_spec[*i] == '{') {
		++*i;
		assert(isdigit(fmt_spec[*i]) && "Expected digit after `{'");
		while (isdigit(fmt_spec[*i])) {
			number = number * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
			assert(number < env->size &&
			       "Cannot reference element past the number of formatting arguments");
		}
		assert(fmt_spec[*i] == '}' && "Expected `}' after number");
		++*i;
		number = (size_t)env->args[number].data;
	}
	/* Literal */
	else if (isdigit(fmt_spec[*i])) {
		while (isdigit(fmt_spec[*i])) {
			number = number * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
		}
	}
	return number;
}

struct format_spec_placeholder
format_parse_placeholder(const char* fmt_spec, size_t* i, const struct format_env* env)
{
	struct format_spec_placeholder placeholder;
	/* Parse from arg list */
	if (fmt_spec[*i] == '{') {
		placeholder.type = 1;
		++*i;
		assert(isdigit(fmt_spec[*i]));
		size_t id = 0;
		while (isdigit(fmt_spec[*i])) {
			id = id * 10 + (size_t)(fmt_spec[*i] - '0');
			++*i;
			assert(id < env->size &&
			       "Cannot reference element past the number of formatting arguments");
		}
		assert(fmt_spec[*i] == '}' && "Expected `}' after number");
		++*i;
		placeholder.str = (const char*)env->args[id].data;
		placeholder.len = strlen(placeholder.str);
		placeholder.width = utf8_len_str(placeholder.str, placeholder.len);
		return placeholder;
	}

	/* Literal, single codepoint only */
	assert(fmt_spec[*i] != '}' && "Expected placeholder");
	const size_t len = format_utf8_len(fmt_spec + *i, 5);
	assert(len <= 5);

	placeholder.type = 0;
	memset(placeholder.codepoint, 0, sizeof placeholder.codepoint);
	placeholder.codepoint[0] = '\0';
	memcpy(placeholder.codepoint, fmt_spec + *i, len);
	placeholder.len = strlen(placeholder.codepoint);
	placeholder.width = utf8_len_str(placeholder.codepoint, placeholder.len);
	*i += len;
	return placeholder;
}

int
format_write_placeholder(struct format_output* output,
                  const struct format_spec_placeholder* placeholder,
                  size_t max_width,
                  int reverse)
{
	/* Write codepoint */
	if (placeholder->type == 0) {
		if (max_width == (size_t)-1)
			max_width = 1;
		/* Width is always 1 */
		for (size_t i = 0; i < max_width; ++i) {
			if (format_output_write(output, placeholder->codepoint, placeholder->len))
				return -1;
		}
		return 0;
	}

	/* Write string content entirely exactly once */
	if (max_width == (size_t)-1) {
		return format_output_write(output, placeholder->str, placeholder->len);
	}

	if (placeholder->width == 0 || max_width == 0)
		return 0;

	const size_t full_cycles = max_width / placeholder->width;
	const size_t remainder = max_width % placeholder->width;

	if (reverse) {
		if (remainder > 0) {
			size_t skip = placeholder->width - remainder;
			size_t off = 0, cps = 0;
			while (cps < skip && off < placeholder->len) {
				const size_t cp = format_utf8_len(placeholder->str + off, placeholder->len - off);
				off += cp ? cp : 1;
				++cps;
			}
			if (format_output_write(output, placeholder->str + off, placeholder->len - off))
				return -1;
		}
		for (size_t i = 0; i < full_cycles; ++i)
			if (format_output_write(output, placeholder->str, placeholder->len))
				return -1;
		return 0;
	}

	for (size_t i = 0; i < full_cycles; ++i)
		if (format_output_write(output, placeholder->str, placeholder->len))
			return -1;
	if (remainder > 0) {
		size_t off = 0, cps = 0;
		while (cps < remainder && off < placeholder->len) {
			const size_t cp = format_utf8_len(placeholder->str + off, placeholder->len - off);
			off += cp ? cp : 1;
			++cps;
		}
		if (format_output_write(output, placeholder->str, off))
			return -1;
	}
	return 0;
}

void
format_parse_alignment(const char* fmt_spec,
                size_t* i,
                const struct format_env* env,
                char* alignment,
                struct format_spec_placeholder* fill,
                const char* default_placeholder)
{
	do {
		/* <align> : parse and return directly */
		if (strchr("<>^", fmt_spec[*i])) {
			*alignment = fmt_spec[*i];
			++*i;
			break;
		}
		/* {N}<align> */
		else if (fmt_spec[*i] == '{') {
			size_t j = *i + 1;
			while (isdigit(fmt_spec[j])) {
				++j;
			}
			if (fmt_spec[j] != '}')
				break;
			++j;
			if (!strchr("<>^", fmt_spec[j]))
				break;
		}
		/* <codepoint><align> */
		else {
			const size_t len = format_utf8_len(fmt_spec + *i, 5);
			if ((len == 1 && fmt_spec[*i] == '#') || !strchr("<>^", fmt_spec[*i + len]))
				break;
		}

		/* Parse placeholder + alignment */
		*fill = format_parse_placeholder(fmt_spec, i, env);
		assert(strchr("<>^", fmt_spec[*i]));
		*alignment = fmt_spec[*i];
		++*i;
		return;
	} while (0);
	fill->type = 1;
	fill->str = default_placeholder;
	fill->len = strlen(fill->str);
	fill->width = utf8_len_str(fill->str, fill->len);
}
