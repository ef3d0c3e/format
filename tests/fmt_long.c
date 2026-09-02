#include "util.h"

void print_buffer(const char *buf, size_t len)
{
	printf(" \"");
	for (size_t i = 0; i < len; ++i)
	{
		if (isprint(buf[i]))
			printf("%c", buf[i]);
		else
			printf("\033[36m%#hhx\033[0m", (unsigned char)buf[i]);
	}
	printf("\" [%zu]\n", len);
}

#define generate(fmt_format_, fmt_printf_, ...) \
do { \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wformat\"") \
	struct format_output out = format_output_buf(); \
	format(&out, fmt_format_, __VA_ARGS__); \
	char *buf; \
	int len = asprintf(&buf, fmt_printf_, __VA_ARGS__); \
	if (out.size != (size_t)len || memcmp(out.data, buf, out.size) != 0) \
	{ \
		printf("Got:\n"); \
		print_buffer(out.data, out.size); \
		printf("Expected:\n"); \
		print_buffer(buf, (size_t)len); \
		cr_assert(0, "test %s", #fmt_format_ "/" #fmt_printf_); \
	} \
	free(buf); \
	format_output_destroy(&out); \
	_Pragma("GCC diagnostic pop") \
} while (false)

#define for_each(type_, varname_, ...) \
	const type_ array_[] = {__VA_ARGS__}; \
	type_ varname_ = array_[0]; \
	for (size_t idx_ = 0; idx_ < sizeof(array_) / sizeof(type_); varname_ = array_[++idx_])

Test(fmt_long, test) {
	for_each(long, x, -1, 0, 1, 64, LONG_MAX, LONG_MIN) {
		generate("{}", "%ld", x);
		generate("|{}|", "|%ld|", x);
		generate("{0}", "%1$ld", x);
		generate("|{0}|", "|%1$ld|", x);
		generate("{0} {0}", "%1$ld %1$ld", x);

		generate("{:x}", "%lx", x);
		generate("|{:x}|", "|%lx|", x);
		generate("{0:x}", "%1$lx", x);
		generate("|{0:x}|", "|%1$lx|", x);
		generate("{0:x} {0:x}", "%1$lx %1$lx", x);

		generate("{:X}", "%lX", x);
		generate("|{:X}|", "|%lX|", x);
		generate("{0:X}", "%1$lX", x);
		generate("|{0:X}|", "|%1$lX|", x);
		generate("{0:X} {0:X}", "%1$lX %1$lX", x);


		generate("{:X}", "%lX", x);
		generate("|{:X}|", "|%lX|", x);
		generate("{0:X}", "%1$lX", x);
		generate("|{0:X}|", "|%1$lX|", x);
		generate("{0:X} {0:X}", "%1$lX %1$lX", x);

		generate("{:b}", "%lb", x);
		generate("|{:b}|", "|%lb|", x);
		generate("{0:b}", "%1$lb", x);
		generate("|{0:b}|", "|%1$lb|", x);
		generate("{0:b} {0:b}", "%1$lb %1$lb", x);

		generate("{:B}", "%lB", x);
		generate("|{:B}|", "|%lB|", x);
		generate("{0:B}", "%1$lB", x);
		generate("|{0:B}|", "|%1$lB|", x);
		generate("{0:B} {0:B}", "%1$lB %1$lB", x);

		/* Alternate */
		generate("{:#}", "%#ld", x);
		generate("|{:#}|", "|%#ld|", x);
		generate("{0:#}", "%1$#ld", x);
		generate("|{0:#}|", "|%1$#ld|", x);
		generate("{0:#} {0:#}", "%1$#ld %1$#ld", x);

		generate("{:#x}", "%#lx", x);
		generate("|{:#x}|", "|%#lx|", x);
		generate("{0:#x}", "%1$#lx", x);
		generate("|{0:#x}|", "|%1$#lx|", x);
		generate("{0:#x} {0:#x}", "%1$#lx %1$#lx", x);

		generate("{:#X}", "%#lX", x);
		generate("|{:#X}|", "|%#lX|", x);
		generate("{0:#X}", "%1$#lX", x);
		generate("|{0:#X}|", "|%1$#lX|", x);
		generate("{0:#X} {0:#X}", "%1$#lX %1$#lX", x);


		generate("{:#X}", "%#lX", x);
		generate("|{:#X}|", "|%#lX|", x);
		generate("{0:#X}", "%1$#lX", x);
		generate("|{0:#X}|", "|%1$#lX|", x);
		generate("{0:#X} {0:#X}", "%1$#lX %1$#lX", x);

		generate("{:#b}", "%#lb", x);
		generate("|{:#b}|", "|%#lb|", x);
		generate("{0:#b}", "%1$#lb", x);
		generate("|{0:#b}|", "|%1$#lb|", x);
		generate("{0:#b} {0:#b}", "%1$#lb %1$#lb", x);

		generate("{:#B}", "%#lB", x);
		generate("|{:#B}|", "|%#lB|", x);
		generate("{0:#B}", "%1$#lB", x);
		generate("|{0:#B}|", "|%1$#lB|", x);
		generate("{0:#B} {0:#B}", "%1$#lB %1$#lB", x);
	}
}
