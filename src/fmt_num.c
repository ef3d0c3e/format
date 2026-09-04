#include "fmt.h"

/** @brief Number format spec */
struct number_spec
{
	/** @brief Fill string */
	struct spec_placeholder fill;
	/**
	 * @brief Alignment character:
	 *  - `<`: Left (default)
	 *  - `>`: Right
	 *  - `^`: Center
	 *  - `0`: Pad with `0`'s
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
 * @brief Parse the format specifier for numbers
 *
 * @param fmt_spec Format specifier
 * @param env Format environment
 *
 * Format: `[align][sign][alternate][zero][width][precision]`
 *
 * @return Parsed @ref number_spec for @p fmt_spec
 */
static inline struct number_spec
parse_number_spec(const char* fmt_spec, const struct format_env* env)
{
	struct number_spec spec = {
		.align = 0,
		.sign = '-', /* Negative only */
		.alternate = 0,
		.width = 0,
		.precision = (size_t)-1,
		.type = 0,
		.left = fmt_spec,
	};
	if (*fmt_spec == '}')
		return spec;

	size_t i = 0;
	/* Parse align */
	parse_alignment(fmt_spec, &i, env, &spec.align, &spec.fill, " ");

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
		assert(spec.align == 0 && "Conflicting alignment detected");
		spec.align = '0';
		++i;
	}
	if (spec.align == 0)
		spec.align = '<'; /* Left-aligned by default */

	/* Parse width */
	spec.width = parse_size(fmt_spec, &i, env);

	/* Parse precision */
	if (fmt_spec[i] == '.') {
		++i;
		spec.precision = parse_size(fmt_spec, &i, env);
	}
	assert((spec.precision == (size_t)-1 || spec.align != '0') &&
	       "Cannot use precision with 0-padding");

	/* Parse type */
	if (fmt_spec[i] != '}')
	{
		spec.type = fmt_spec[i++];
		assert(strchr("xXbB", spec.type) && "Invalid display type");
	}

	spec.left = fmt_spec + i;
	return spec;
}

/**
 * @brief Write number buffer with alignment
 *
 * @param output Destination
 * @parram spec Format specifier
 * @param buf Buffer to write
 * @param len Length of @p buf
 * @param num_len Length of the numeric part of @p buf (number of digits)
 * @param zero_pos Position at which to insert `0`'s for precision and 0-padding
 */
static inline int
write_aligned(struct format_output* output,
              const struct number_spec* spec,
              const char* buf,
              size_t len,
              size_t num_len,
              size_t zero_pos)
{
	assert(zero_pos < len);

	size_t digit_len = len - zero_pos; /* natural digit byte count */
	size_t precision_zeros = 0;
	if (spec->precision != (size_t)-1) {
		precision_zeros = spec->precision > num_len ? spec->precision - num_len : 0;
		digit_len = num_len; /* only actually emit num_len digit bytes */
	}
	const size_t content_len = zero_pos + precision_zeros + digit_len;

	size_t left = 0, right = 0;
	switch (spec->align) {
		case '^':
			left = (spec->width > content_len ? spec->width - content_len : 0);
			right = left / 2;
			left -= right;
			break;
		case '<':
			right = spec->width > content_len ? spec->width - content_len : 0;
			break;
		case '>':
			left = spec->width > content_len ? spec->width - content_len : 0;
			break;
		case '0':
			break;
		default:
			__builtin_unreachable();
	}

	if (write_placeholder(output, &spec->fill, left, 0))
		return -1;

	if (spec->align == '0') {
		if (format_output_write(output, buf, zero_pos))
			return -1;
		const size_t zero_count = spec->width > len ? (spec->width - len) : 0;
		for (size_t i = 0; i < zero_count; ++i)
			if (format_output_write(output, "0", 1))
				return -1;
		if (format_output_write(output, buf + zero_pos, len - zero_pos))
			return -1;
	} else if (spec->precision != (size_t)-1) {
		if (format_output_write(output, buf, zero_pos))
			return -1;
		for (size_t i = 0; i < precision_zeros; ++i)
			if (format_output_write(output, "0", 1))
				return -1;
		if (format_output_write(output, buf + zero_pos, digit_len))
			return -1;
	} else {
		if (format_output_write(output, buf, len))
			return -1;
	}

	if (write_placeholder(output, &spec->fill, right, 1))
		return -1;
	return 0;
}

#define format_unsigned_value(type__)                                                            \
	do {                                                                                         \
		struct number_spec spec = parse_number_spec(fmt_spec, env);                              \
		assert(spec.left[0] == '}' && "Leftover content in format specifier");                   \
                                                                                                 \
		const type__ val = (type__)env->args[idx].data;                                          \
		char buf[sizeof(type__) * 8 + 16];                                                       \
                                                                                                 \
		/* Get base */                                                                           \
		const type__ base = (tolower(spec.type) == 'x')   ? 16                                   \
		                    : (tolower(spec.type) == 'b') ? 2                                    \
		                                                  : 10;                                  \
                                                                                                 \
		/* Compute length in base */                                                             \
		size_t len = 0;                                                                          \
		for (type__ x = val; x; x /= base)                                                       \
			++len;                                                                               \
		if (val == 0)                                                                            \
			len = 1;                                                                             \
		const size_t num_len = (val == 0 && spec.precision == 0) ? 0 : len;                      \
                                                                                                 \
		size_t start = 0;                                                                        \
                                                                                                 \
		/* Prefix */                                                                             \
		if (spec.alternate && spec.type != '\0' && val != 0) {                                   \
			buf[start++] = '0';                                                                  \
			buf[start++] = spec.type;                                                            \
			len += 2;                                                                            \
		}                                                                                        \
                                                                                                 \
		/* Value */                                                                              \
		if (val == 0)                                                                            \
			buf[start] = '0';                                                                    \
		else {                                                                                   \
			type__ x = val;                                                                      \
			for (size_t i = 0; x; ++i) {                                                         \
				const int d = (int)(x % base);                                                   \
				x /= base;                                                                       \
				switch (spec.type) {                                                             \
					case 'x':                                                                    \
						buf[len - i - 1] = FORMAT_HEX[d];                                \
						break;                                                                   \
					case 'X':                                                                    \
						buf[len - i - 1] = FORMAT_HEX_CAPITAL[d];                                \
						break;                                                                   \
					default:                                                                     \
						buf[len - i - 1] = (char)('0' + d);                                      \
						break;                                                                   \
				}                                                                                \
			}                                                                                    \
		}                                                                                        \
                                                                                                 \
		return write_aligned(output, &spec, buf, len, num_len, start);                           \
	} while (0);

#define format_signed_value(type__, unsigned_type__)                                             \
	do {                                                                                         \
		struct number_spec spec = parse_number_spec(fmt_spec, env);                              \
		assert(spec.left[0] == '}' && "Leftover content in format specifier");                   \
                                                                                                 \
		const type__ val = (type__)env->args[idx].data;                                          \
		char buf[sizeof(type__) * 8 + 16];                                                       \
                                                                                                 \
		/* Get base */                                                                           \
		const type__ base = (tolower(spec.type) == 'x')   ? 16                                   \
		                    : (tolower(spec.type) == 'b') ? 2                                    \
		                                                  : 10;                                  \
		if (base != 10) {                                                                        \
			format_unsigned_value(unsigned_type__);                                              \
		}                                                                                        \
                                                                                                 \
		/* Compute length in base */                                                             \
		size_t len = 0;                                                                          \
		for (type__ x = val; x; x /= base)                                                       \
			++len;                                                                               \
		if (val == 0)                                                                            \
			len = 1;                                                                             \
		const size_t num_len = (val == 0 && spec.precision == 0) ? 0 : len;                      \
                                                                                                 \
		/* Sign */                                                                               \
		size_t start = 0;                                                                        \
		if (val < 0) {                                                                           \
			++len;                                                                               \
			buf[start++] = '-';                                                                  \
		} else if (spec.sign != '-') {                                                           \
			++len;                                                                               \
			buf[start++] = spec.sign == '+' ? '+' : ' ';                                         \
		}                                                                                        \
                                                                                                 \
		/* Prefix */                                                                             \
		if (spec.alternate && spec.type != '\0' && val != 0) {                                   \
			buf[start++] = '0';                                                                  \
			buf[start++] = spec.type;                                                            \
			len += 2;                                                                            \
		}                                                                                        \
                                                                                                 \
		/* Value */                                                                              \
		if (val == 0)                                                                            \
			buf[start] = '0';                                                                    \
		else {                                                                                   \
			type__ x = val;                                                                      \
			for (size_t i = 0; x; ++i) {                                                         \
				const int d = val >= 0 ? (int)(x % base) : -(int)(x % base);                     \
				x /= base;                                                                       \
				switch (spec.type) {                                                             \
					case 'x':                                                                    \
						buf[len - i - 1] = FORMAT_HEX[d];                                \
						break;                                                                   \
					case 'X':                                                                    \
						buf[len - i - 1] = FORMAT_HEX_CAPITAL[d];                                \
						break;                                                                   \
					default:                                                                     \
						buf[len - i - 1] = (char)('0' + d);                                      \
						break;                                                                   \
				}                                                                                \
			}                                                                                    \
		}                                                                                        \
                                                                                                 \
		return write_aligned(output, &spec, buf, len, num_len, start);                           \
	} while (0)

#define dispatch_signed(type__)                                                                  \
	do {                                                                                         \
		switch (sizeof(type__)) {                                                                \
			case 8:                                                                              \
				format_signed_value(int64_t, uint64_t);                                          \
			case 4:                                                                              \
				format_signed_value(int32_t, uint32_t);                                          \
			case 2:                                                                              \
				format_signed_value(int16_t, uint16_t);                                          \
			case 1:                                                                              \
				format_signed_value(int8_t, uint8_t);                                            \
			default:                                                                             \
				__builtin_unreachable();                                                         \
		}                                                                                        \
	} while (0)

#define dispatch_unsigned(type__)                                                                \
	do {                                                                                         \
		switch (sizeof(type__)) {                                                                \
			case 8:                                                                              \
				format_unsigned_value(uint64_t);                                                 \
			case 4:                                                                              \
				format_unsigned_value(uint32_t);                                                 \
			case 2:                                                                              \
				format_unsigned_value(uint16_t);                                                 \
			case 1:                                                                              \
				format_unsigned_value(uint8_t);                                                  \
			default:                                                                             \
				__builtin_unreachable();                                                         \
		}                                                                                        \
	} while (0)

int
format_fmt_long_long(struct format_output* output,
                     const char* fmt_spec,
                     const struct format_env* env,
                     size_t idx)
{
	dispatch_signed(long long);
}

int
format_fmt_unsigned_long_long(struct format_output* output,
                              const char* fmt_spec,
                              const struct format_env* env,
                              size_t idx)
{
	dispatch_unsigned(unsigned long long);
}

int
format_fmt_long(struct format_output* output,
                const char* fmt_spec,
                const struct format_env* env,
                size_t idx)
{
	dispatch_signed(long);
}

int
format_fmt_unsigned_long(struct format_output* output,
                         const char* fmt_spec,
                         const struct format_env* env,
                         size_t idx)
{
	dispatch_unsigned(unsigned long);
}

int
format_fmt_int(struct format_output* output,
               const char* fmt_spec,
               const struct format_env* env,
               size_t idx)
{
	dispatch_signed(int);
}

int
format_fmt_unsigned_int(struct format_output* output,
                        const char* fmt_spec,
                        const struct format_env* env,
                        size_t idx)
{
	dispatch_unsigned(unsigned int);
}

int
format_fmt_short(struct format_output* output,
                 const char* fmt_spec,
                 const struct format_env* env,
                 size_t idx)
{
	dispatch_signed(short);
}

int
format_fmt_unsigned_short(struct format_output* output,
                          const char* fmt_spec,
                          const struct format_env* env,
                          size_t idx)
{
	dispatch_unsigned(unsigned short);
}

int
format_fmt_signed_char(struct format_output* output,
                       const char* fmt_spec,
                       const struct format_env* env,
                       size_t idx)
{
	dispatch_signed(signed char);
}

int
format_fmt_unsigned_char(struct format_output* output,
                         const char* fmt_spec,
                         const struct format_env* env,
                         size_t idx)
{
	dispatch_unsigned(unsigned char);
}
