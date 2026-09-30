#include "util.h"

#define SIGNED_TESTS(type_, length_, min_, max_)                                                 \
	for_each(type_, x, (type_)-1, (type_)0, (type_)1, (type_)64, (type_)max_, (type_)min_) {    \
		test_printf("{}", "%" length_ "d", x);                                                   \
		test_printf("{:x}", "%" length_ "x", x);                                                 \
		test_printf("{:X}", "%" length_ "X", x);                                                 \
		test_printf("{:b}", "%" length_ "b", x);                                                 \
		test_printf("{:B}", "%" length_ "B", x);                                                 \
		test_printf("{:#x}", "%#" length_ "x", x);                                               \
		test_printf("{:#X}", "%#" length_ "X", x);                                               \
		test_printf("{:#b}", "%#" length_ "b", x);                                               \
		test_printf("{:#B}", "%#" length_ "B", x);                                               \
		test_printf("{:05}", "%05" length_ "d", x);                                               \
		test_printf("{:05x}", "%05" length_ "x", x);                                              \
		test_printf("{:.5}", "%.5" length_ "d", x);                                              \
		test_printf("{:+.5}", "%+.5" length_ "d", x);                                            \
	}

#define UNSIGNED_TESTS(type_, length_, max_)                                                     \
	for_each(type_, x, (type_)0, (type_)1, (type_)64, (type_)max_) {                            \
		test_printf("{}", "%" length_ "u", x);                                                   \
		test_printf("{:x}", "%" length_ "x", x);                                                 \
		test_printf("{:X}", "%" length_ "X", x);                                                 \
		test_printf("{:b}", "%" length_ "b", x);                                                 \
		test_printf("{:B}", "%" length_ "B", x);                                                 \
		test_printf("{:#x}", "%#" length_ "x", x);                                               \
		test_printf("{:#X}", "%#" length_ "X", x);                                               \
		test_printf("{:#b}", "%#" length_ "b", x);                                               \
		test_printf("{:#B}", "%#" length_ "B", x);                                               \
		test_printf("{:05}", "%05" length_ "u", x);                                               \
		test_printf("{:.5}", "%.5" length_ "u", x);                                              \
	}

Test(fmt_int, format)
{
	SIGNED_TESTS(int, "", INT_MIN, INT_MAX)
}

Test(fmt_long_long, format)
{
	SIGNED_TESTS(long long, "ll", LLONG_MIN, LLONG_MAX)
}

Test(fmt_short, format)
{
	SIGNED_TESTS(short, "h", SHRT_MIN, SHRT_MAX)
}

Test(fmt_signed_char, format)
{
	SIGNED_TESTS(signed char, "hh", SCHAR_MIN, SCHAR_MAX)
}

Test(fmt_unsigned_int, format)
{
	UNSIGNED_TESTS(unsigned int, "", UINT_MAX)
}

Test(fmt_unsigned_long_long, format)
{
	UNSIGNED_TESTS(unsigned long long, "ll", ULLONG_MAX)
}

Test(fmt_unsigned_short, format)
{
	UNSIGNED_TESTS(unsigned short, "h", USHRT_MAX)
}

Test(fmt_unsigned_char, format)
{
	UNSIGNED_TESTS(unsigned char, "hh", UCHAR_MAX)
}