#include "fmt.h"
#include <ctype.h>
#include <string.h>

static inline int
next(const char* fmt, size_t i, size_t* start, size_t* end)
{
	int balance = 0;

	*start = *end = 0;
	for (; fmt[i]; ++i) {
		/* Skip escaped braces */
		if (balance == 0 && fmt[i] == '{' && fmt[i + 1] == '{') {
			++i;
			continue;
		}
		if (balance == 0 && fmt[i] == '}' && fmt[i + 1] == '}') {
			++i;
			continue;
		}

		if (fmt[i] == '}') {
			if (balance == 1)
				*end = i;
			--balance;
			assert(balance >= 0);
		} else if (fmt[i] == '{') {
			if (balance == 0)
				*start = i;
			++balance;
		}
		if (!balance && *end != 0)
			return 1;
	}
	return 0;
}

static inline void
write_escaped(struct format_output* output, const char* buf, size_t len)
{
	// TODO: Error
	for (size_t i = 0; i < len;) {
		assert(buf[i] != 0);
		/* Emit one */
		if (buf[i] == '{' || buf[i] == '}') {
			assert(buf[i] == buf[i + 1]);
			format_output_write(output, buf + i, 1);
			i += 2;
		} else {
			format_output_write(output, buf + i, 1);
			++i;
		}
	}
}

/**
 * @brief Write style to output
 *
 * @param output Output to apply style to
 * @param fg Foreground color to apply
 * @param bg Background color to apply
 * @param style Style to apply
 *
 * @return `0` on success, `-1` on errors
 */
static inline int
write_style(struct format_output* output,
                    format_color fg,
                    format_color bg,
                    enum format_output_style style)
{
	if (style == kFormatStyleReset) {
		return format_output_write(output, "\033[0m", 4);
	}

	char buf[256];
	size_t i = 0;
	if (fg != (format_color)~0U && fg != output->fg) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '3';
		buf[i++] = '8';
		buf[i++] = ';';
		buf[i++] = '2';
		buf[i++] = ';';
		const uint8_t r = (fg >> 16) & 0xFF;
		if (r > 100)
			buf[i++] = (char)('0' + (r / 100));
		if (r > 10)
			buf[i++] = (char)('0' + ((r / 10) % 10));
		buf[i++] = (char)('0' + (r % 10));
		buf[i++] = ';';
		const uint8_t g = (fg >> 8) & 0xFF;
		if (g > 100)
			buf[i++] = (char)('0' + (g / 100));
		if (g > 10)
			buf[i++] = (char)('0' + ((g / 10) % 10));
		buf[i++] = (char)('0' + (g % 10));
		buf[i++] = ';';
		const uint8_t b = fg & 0xFF;
		if (b > 100)
			buf[i++] = (char)('0' + (b / 100));
		if (b > 10)
			buf[i++] = (char)('0' + ((b / 10) % 10));
		buf[i++] = (char)('0' + (b % 10));
		buf[i++] = 'm';
	}
	if (bg != (format_color)~0U && bg != output->bg) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '4';
		buf[i++] = '8';
		buf[i++] = ';';
		buf[i++] = '2';
		buf[i++] = ';';
		const uint8_t r = (bg >> 16) & 0xFF;
		if (r > 100)
			buf[i++] = (char)('0' + (r / 100));
		if (r > 10)
			buf[i++] = (char)('0' + ((r / 10) % 10));
		buf[i++] = (char)('0' + (r % 10));
		buf[i++] = ';';
		const uint8_t g = (bg >> 8) & 0xFF;
		if (g > 100)
			buf[i++] = (char)('0' + (g / 100));
		if (g > 10)
			buf[i++] = (char)('0' + ((g / 10) % 10));
		buf[i++] = (char)('0' + (g % 10));
		buf[i++] = ';';
		const uint8_t b = bg & 0xFF;
		if (b > 100)
			buf[i++] = (char)('0' + (b / 100));
		if (b > 10)
			buf[i++] = (char)('0' + ((b / 10) % 10));
		buf[i++] = (char)('0' + (b % 10));
		buf[i++] = 'm';
	}
	if ((style & kFormatStyleBold) && !(output->style & kFormatStyleBold)) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '1';
		buf[i++] = 'm';
	}
	if ((style & kFormatStyleItalic) && !(output->style & kFormatStyleItalic)) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '3';
		buf[i++] = 'm';
	}
	if ((style & kFormatStyleUnderline) && !(output->style & kFormatStyleUnderline)) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '4';
		buf[i++] = 'm';
	}
	if ((style & kFormatStyleCrossed) && !(output->style & kFormatStyleCrossed)) {
		buf[i++] = '\033';
		buf[i++] = '[';
		buf[i++] = '9';
		buf[i++] = 'm';
	}
	if (i > 0)
		return format_output_write(output, buf, i);
	return 0;
}

/**
 * @brief Parse color value
 *
 * @param fmt Format string
 * @param i Position in @p fmt, will advance
 *
 * @return Parser color in @p fmt at @p i
 */
static inline format_color
parse_color(const char* fmt, size_t* i)
{
	format_color color = 0;
	while (strchr("0123456789abcdef", tolower(fmt[*i]))) {
		if (isdigit(fmt[*i]))
			color = color * 16 + (format_color)(fmt[*i] - '0');
		else
			color = color * 16 + 10 + (format_color)(tolower(fmt[*i]) - 'a');
		++*i;
	}
	return color;
}

static inline void
fmt_style(struct format_output* output, const char* fmt, size_t* i)
{
	format_color fg = (format_color)~0U;
	format_color bg = (format_color)~0U;
	enum format_output_style style = kFormatStyleNone;

	while (1) {
		if (fmt[*i] == 'f' && fmt[*i + 1] == 'g' && fmt[*i + 2] == '#') {
			assert(fg == (format_color)~0U);
			*i += 3;
			fg = parse_color(fmt, i);
			if (fmt[*i] == '}')
				break;
			assert(fmt[*i] == ' ' || fmt[*i] == '\t');
			while (strchr(" \t", fmt[*i]))
				++*i;
		} else if (fmt[*i] == 'b' && fmt[*i + 1] == 'g' && fmt[*i + 2] == '#') {
			assert(bg == (format_color)~0U);
			*i += 3;
			bg = parse_color(fmt, i);
			if (fmt[*i] == '}')
				break;
			assert(fmt[*i] == ' ' || fmt[*i] == '\t');
			while (strchr(" \t", fmt[*i]))
				++*i;
		} else if (fmt[*i] == '/') {
			assert(style == kFormatStyleNone);
			++*i;

			if (fmt[*i] == '0') {
				style = kFormatStyleReset;
				++*i;
			} else {
				while (strchr("bciu", fmt[*i])) {
					style |= (fmt[*i] == 'b') * kFormatStyleBold |
					         (fmt[*i] == 'i') * kFormatStyleItalic |
					         (fmt[*i] == 'u') * kFormatStyleUnderline |
					         (fmt[*i] == 'c') * kFormatStyleCrossed;
					++*i;
				}
			}

			if (fmt[*i] == '}')
				break;
			assert(fmt[*i] == ' ' || fmt[*i] == '\t');
			while (strchr(" \t", fmt[*i]))
				++*i;
		} else {
			assert(0 && "Invalid style");
			__builtin_unreachable();
		}
	}

	write_style(output, fg, bg, style);
}

void
format_args(struct format_output* output, const char* fmt, const struct format_env env)
{
	// TODO: Err handling
	size_t cur_positional = 0;
	for (size_t i = 0; fmt[i];) {
		size_t start, end;
		if (!next(fmt, i, &start, &end)) {
			/* Nothing left */
			write_escaped(output, fmt + i, strlen(fmt + i));
			break;
		}
		write_escaped(output, fmt + i, start - i);
		assert(start <= end);
		i = start + 1;
		if (fmt[i] == '{') {
			format_output_write(output, "{", 1);
			++i;
			continue;
		}
		assert(fmt[i] != '{');

		/* Custom style */
		if (fmt[i] == 'f' || fmt[i] == 'b' || fmt[i] == '/') {
			fmt_style(output, fmt, &i);
			assert(fmt[i] == '}');
			++i;
			continue;
		}

		/* Parse index */
		size_t index = 0;
		if (isdigit(fmt[i])) {
			while (isdigit(fmt[i])) {
				index = index * 10 + (size_t)(fmt[i] - '0');
				++i;
				assert(index < 4096); /* Expansion macro would have stopped before that */
			}
		} else
			index = cur_positional++;
		assert(index < env.size);

		/* Find delimiters */
		const char* sep = strchr(fmt + i, ':');
		if (sep != NULL)
			++sep;
		if (sep == NULL || sep > fmt + end)
			sep = fmt + end;

		/* Call formatter */
		if (env.args[index].type == kFormatScalar)
		{
			assert(env.args[index].formatter != NULL);
			env.args[index].formatter(output, sep, &env, index);
		}
		else if (env.args[index].type == kFormatCollection)
		{
			format_fmt_collection(output, sep, &env, index);
		}
		i = end + 1;
	}
}
