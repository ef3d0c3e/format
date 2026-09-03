#include "fmt.h"

struct string_spec
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
	/** @brief Minimum field width */
	size_t width;
	/** @brief Toggle quoted mode */
	int quoted;
	/** @brief Left quote codepoint */
	char left_quote[5];
	/** @brief Right quote codepoint */
	char right_quote[5];
	/** @brief Field precision */
	size_t precision;
	/**
	 * @brief Display type
	 *  - 's' Display characters as is (default)
	 *  - '?' Display non printable using common escape sequences or as `\xXX`
	 *  - 'x' Display non printables as `0xXX`
	 */
	char type;
	/** @brief Leftover, whatever is left after parsing the previous fields */
	const char* left;
};

static inline struct string_spec
parse_string_spec(const char* fmt_spec, const struct fmt_env* env)
{
	struct string_spec spec = {
		.fill = { ' ', 0, 0, 0, 0 }, /* Space */
		.align = '<',                /* Left-aligned by default */
		.width = 0,
		.quoted = 0,
		.left_quote = { 0, 0, 0, 0, 0 },
		.right_quote = { 0, 0, 0, 0, 0 },
		.precision = (size_t)-1, /* Sentinel */
		.type = 's',
		.left = fmt_spec,
	};

	if (*fmt_spec == '}')
		return spec;

	size_t i = 0;
	/* Parse align */
	const size_t len =
	  utf8_len(fmt_spec, 5); /* Get the width of the alignment character codepoint */
	assert(len <= 5);
	if (strchr("<>^", fmt_spec[len])) /* Custom character */
	{
		spec.align = fmt_spec[len];
		strncpy((char*)spec.fill, fmt_spec, len);
		i = len + 1;
	} else if (strchr("<>^", fmt_spec[i])) /* Default character */
	{
		spec.align = fmt_spec[i++];
	}

	/* Parse width */
	spec.width = parse_size(fmt_spec, &i, env);

	/* Parse quotes */
	if (fmt_spec[i] == '#') {
		++i;
		spec.quoted = 1;
		assert(fmt_spec[i] != '}');
		size_t len = utf8_len(fmt_spec + i, 5);
		strncpy(spec.left_quote, fmt_spec + i, len);
		i += len;
		assert(fmt_spec[i] != '}');
		len = utf8_len(fmt_spec + i, 5);
		strncpy(spec.right_quote, fmt_spec + i, len);
		i += len;
	}

	/* Parse precision */
	if (fmt_spec[i] == '.') {
		++i;
		spec.precision = parse_size(fmt_spec, &i, env);
	}

	/* Parse type */
	if (fmt_spec[i] != '}') {
		spec.type = fmt_spec[i++];
		assert(strchr("s?x", spec.type) && "Invalid display type");
	}

	spec.left = fmt_spec + i;
	return spec;
}

int
format_fmt_str(struct format_output* output,
               const char* fmt_spec,
               const struct fmt_env* env,
               size_t idx)
{
	struct string_spec spec = parse_string_spec(fmt_spec, env);
	assert(spec.left[0] == '}' && "Leftover content in format specifier");

	const char* val = (const char*)env->args[idx].data;

	/* Number of bytes to display */
	size_t len = 0;
	if (spec.precision == (size_t)-1)
		len = strlen(val);
	else
		len = strnlen(val, spec.precision);

	/* Compute display width */
	size_t width = 0;
	if (spec.quoted)
		width += 2;
	for (size_t i = 0; i < len;) {
		const size_t cp = utf8_len(val + i, len - i);
		if (cp == 1) {
			if (isprint(val[i]) || val[i] == '\t')
				width += 1;
			else if (strchr("\a\b\n\v\f\r", val[i]) && spec.type == '?')
				width += 2;
			else
				width += spec.type == 's' ? 0 : 4 /* \xXX or 0xXX */;
		} else if (cp > 1)
			width += 1;
		i += cp ? cp : 1;
	}

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
			__builtin_unreachable();
	}

	const size_t fill_len = strnlen(spec.fill, 5);
	/* Left spacing */
	for (size_t i = 0; i < left; ++i)
		if (format_output_write(output, spec.fill, fill_len))
			return -1;
	/* Left quote */
	if (spec.quoted) {
		if (format_output_write(output, spec.left_quote, strnlen(spec.left_quote, 5)))
			return -1;
	}

	/* Content */
	char buf[16];
	for (size_t i = 0; i < len;) {
		const size_t cp = utf8_len(val + i, len - i);
		if (cp == 0)
		{
			/* Write as-is */
			if (format_output_write(output, val + i, 1))
				return -1;
		}
		else if (cp == 1) {
			if (isprint(val[i]) || val[i] == '\t') {
				if (format_output_write(output, val + i, 1))
					return -1;
			} else if (strchr("\a\b\n\v\f\r", val[i]) && spec.type == '?') {
				buf[0] = '\\';
				switch (val[i]) {
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
						__builtin_unreachable();
				}
				if (format_output_write(output, buf, 2))
					return -1;
			} else {
				switch (spec.type) {
					case 's':
						if (format_output_write(output, val + i, 1))
							return -1;
						break;
					case '?':
						buf[0] = '\\';
						buf[1] = 'x';
						buf[2] = FORMAT_HEX[(unsigned char)val[i] / 16];
						buf[3] = FORMAT_HEX[(unsigned char)val[i] % 16];
						if (format_output_write(output, buf, 4))
							return -1;
						break;
					case 'x':
						buf[0] = '0';
						buf[1] = 'x';
						buf[2] = FORMAT_HEX[(unsigned char)val[i] / 16];
						buf[3] = FORMAT_HEX[(unsigned char)val[i] % 16];
						if (format_output_write(output, buf, 4))
							return -1;
						break;
					default:
						__builtin_unreachable();
				}
			}
		} else if (cp > 1)
		{
			if (format_output_write(output, val + i, cp))
				return -1;
		}
		i += cp ? cp : 1;
	}

	/* Right quote */
	if (spec.quoted) {
		if (format_output_write(output, spec.right_quote, strnlen(spec.right_quote, 5)))
			return -1;
	}
	/* Right spacing */
	for (size_t i = 0; i < right; ++i)
		if (format_output_write(output, spec.fill, fill_len))
			return -1;

	return 0;
}
