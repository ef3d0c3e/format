#include "util.h"

Test(fmt_unsigned_long, format) {
	for_each(unsigned long, x, 0, 1, 64, LONG_MAX, ULONG_MAX) {
		test_printf("{}", "%lu", x);
		test_printf("|{}|", "|%lu|", x);
		test_printf("{0}", "%1$lu", x);
		test_printf("|{0}|", "|%1$lu|", x);
		test_printf("{0} {0}", "%1$lu %1$lu", x);

		test_printf("{:x}", "%lx", x);
		test_printf("|{:x}|", "|%lx|", x);
		test_printf("{0:x}", "%1$lx", x);
		test_printf("|{0:x}|", "|%1$lx|", x);
		test_printf("{0:x} {0:x}", "%1$lx %1$lx", x);

		test_printf("{:X}", "%lX", x);
		test_printf("|{:X}|", "|%lX|", x);
		test_printf("{0:X}", "%1$lX", x);
		test_printf("|{0:X}|", "|%1$lX|", x);
		test_printf("{0:X} {0:X}", "%1$lX %1$lX", x);


		test_printf("{:X}", "%lX", x);
		test_printf("|{:X}|", "|%lX|", x);
		test_printf("{0:X}", "%1$lX", x);
		test_printf("|{0:X}|", "|%1$lX|", x);
		test_printf("{0:X} {0:X}", "%1$lX %1$lX", x);

		test_printf("{:b}", "%lb", x);
		test_printf("|{:b}|", "|%lb|", x);
		test_printf("{0:b}", "%1$lb", x);
		test_printf("|{0:b}|", "|%1$lb|", x);
		test_printf("{0:b} {0:b}", "%1$lb %1$lb", x);

		test_printf("{:B}", "%lB", x);
		test_printf("|{:B}|", "|%lB|", x);
		test_printf("{0:B}", "%1$lB", x);
		test_printf("|{0:B}|", "|%1$lB|", x);
		test_printf("{0:B} {0:B}", "%1$lB %1$lB", x);

		/* Alternate */
		test_printf("{:#}", "%#lu", x);
		test_printf("|{:#}|", "|%#lu|", x);
		test_printf("{0:#}", "%1$#lu", x);
		test_printf("|{0:#}|", "|%1$#lu|", x);
		test_printf("{0:#} {0:#}", "%1$#lu %1$#lu", x);

		test_printf("{:#x}", "%#lx", x);
		test_printf("|{:#x}|", "|%#lx|", x);
		test_printf("{0:#x}", "%1$#lx", x);
		test_printf("|{0:#x}|", "|%1$#lx|", x);
		test_printf("{0:#x} {0:#x}", "%1$#lx %1$#lx", x);

		test_printf("{:#X}", "%#lX", x);
		test_printf("|{:#X}|", "|%#lX|", x);
		test_printf("{0:#X}", "%1$#lX", x);
		test_printf("|{0:#X}|", "|%1$#lX|", x);
		test_printf("{0:#X} {0:#X}", "%1$#lX %1$#lX", x);


		test_printf("{:#X}", "%#lX", x);
		test_printf("|{:#X}|", "|%#lX|", x);
		test_printf("{0:#X}", "%1$#lX", x);
		test_printf("|{0:#X}|", "|%1$#lX|", x);
		test_printf("{0:#X} {0:#X}", "%1$#lX %1$#lX", x);

		test_printf("{:#b}", "%#lb", x);
		test_printf("|{:#b}|", "|%#lb|", x);
		test_printf("{0:#b}", "%1$#lb", x);
		test_printf("|{0:#b}|", "|%1$#lb|", x);
		test_printf("{0:#b} {0:#b}", "%1$#lb %1$#lb", x);

		test_printf("{:#B}", "%#lB", x);
		test_printf("|{:#B}|", "|%#lB|", x);
		test_printf("{0:#B}", "%1$#lB", x);
		test_printf("|{0:#B}|", "|%1$#lB|", x);
		test_printf("{0:#B} {0:#B}", "%1$#lB %1$#lB", x);
	}
}
