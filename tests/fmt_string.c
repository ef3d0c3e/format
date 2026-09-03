#include "util.h"

Test(fmt_string, format) {
	const char *s = "Hello";
	test_printf("{}", "%s", s);
	test_printf("{}", "%s", "abcdef");
}
