#include "fmt.h"

size_t
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

size_t
parse_size(const char* fmt_spec, size_t* i, const struct fmt_env* env)
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
		// TODO enforce variant
		size = (size_t)env->args[size].data;
		assert(size <= 16384 && "Size cannot exceed 16384");
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
