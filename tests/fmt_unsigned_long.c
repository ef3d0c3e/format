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

Test(fmt_unsigned_long, zero) {
	for_each(unsigned long, x, 0, 1, 64, 123456, LONG_MAX, ULONG_MAX) {
		test_printf("{:05}", "%05lu", x);
		test_printf("{:010}", "%010lu", x);
		test_printf("{:020}", "%020lu", x);
		test_printf("{:00}", "%00lu", x);

		test_printf("{:05x}", "%05lx", x);
		test_printf("{:010x}", "%010lx", x);
		test_printf("{:020x}", "%020lx", x);
		test_printf("{:00x}", "%00lx", x);

		test_printf("{:05X}", "%05lX", x);
		test_printf("{:010X}", "%010lX", x);
		test_printf("{:020X}", "%020lX", x);
		test_printf("{:00X}", "%00lX", x);

		test_printf("{:05b}", "%05lb", x);
		test_printf("{:010b}", "%010lb", x);
		test_printf("{:020b}", "%020lb", x);
		test_printf("{:00b}", "%00lb", x);

		test_printf("{:05B}", "%05lB", x);
		test_printf("{:010B}", "%010lB", x);
		test_printf("{:020B}", "%020lB", x);
		test_printf("{:00B}", "%00lB", x);

		test_printf("{:#05x}", "%#05lx", x);
		test_printf("{:#010x}", "%#010lx", x);
		test_printf("{:#020x}", "%#020lx", x);
		test_printf("{:#00x}", "%#00lx", x);

		test_printf("{:#05X}", "%#05lX", x);
		test_printf("{:#010X}", "%#010lX", x);
		test_printf("{:#020X}", "%#020lX", x);
		test_printf("{:#00X}", "%#00lX", x);

		test_printf("{:#05b}", "%#05lb", x);
		test_printf("{:#010b}", "%#010lb", x);
		test_printf("{:#020b}", "%#020lb", x);
		test_printf("{:#00b}", "%#00lb", x);

		test_printf("{:#05B}", "%#05lB", x);
		test_printf("{:#010B}", "%#010lB", x);
		test_printf("{:#020B}", "%#020lB", x);
		test_printf("{:#00B}", "%#00lB", x);
	}

	// 0
	test_manual("{:-05}", "00000", 0UL);
	test_manual("{:-010}", "0000000000", 0UL);
	test_manual("{:-020}", "00000000000000000000", 0UL);
	test_manual("{:-00}", "0", 0UL);

	// 1
	test_manual("{:-05}", "00001", 1UL);
	test_manual("{:-010}", "0000000001", 1UL);
	test_manual("{:-020}", "00000000000000000001", 1UL);
	test_manual("{:-00}", "1", 1UL);

	// 64
	test_manual("{:-05}", "00064", 64UL);
	test_manual("{:-010}", "0000000064", 64UL);
	test_manual("{:-020}", "00000000000000000064", 64UL);
	test_manual("{:-00}", "64", 64UL);
}

Test(fmt_unsigne_long, align) {
	// 0
	test_manual("{:>1}", "0", 0UL);
	test_manual("{:>5}", "    0", 0UL);
	test_manual("{:>10}", "         0", 0UL);
	test_manual("{:>15}", "              0", 0UL);
	test_manual("{:>20}", "                   0", 0UL);

	test_manual("{:<1}", "0", 0UL);
	test_manual("{:<5}", "0    ", 0UL);
	test_manual("{:<10}", "0         ", 0UL);
	test_manual("{:<15}", "0              ", 0UL);
	test_manual("{:<20}", "0                   ", 0UL);

	test_manual("{:^1}", "0", 0UL);
	test_manual("{:^5}", "  0  ", 0UL);
	test_manual("{:^10}", "     0    ", 0UL);
	test_manual("{:^15}", "       0       ", 0UL);
	test_manual("{:^20}", "          0         ", 0UL);

	test_manual("{:か>1}", "0", 0UL);
	test_manual("{:か>5}", "かかかか0", 0UL);
	test_manual("{:か>10}", "かかかかかかかかか0", 0UL);
	test_manual("{:か>15}", "かかかかかかかかかかかかかか0", 0UL);
	test_manual("{:か>20}", "かかかかかかかかかかかかかかかかかかか0", 0UL);

	test_manual("{:🎅<1}", "0", 0UL);
	test_manual("{:🎅<5}", "0🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<10}", "0🎅🎅🎅🎅🎅🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<15}", "0🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<20}", "0🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 0UL);

	test_manual("{:~^1}", "0", 0UL);
	test_manual("{:~^5}", "~~0~~", 0UL);
	test_manual("{:~^10}", "~~~~~0~~~~", 0UL);
	test_manual("{:~^15}", "~~~~~~~0~~~~~~~", 0UL);
	test_manual("{:~^20}", "~~~~~~~~~~0~~~~~~~~~", 0UL);

	// 1
	test_manual("{:>1}", "1", 1UL);
	test_manual("{:>5}", "    1", 1UL);
	test_manual("{:>10}", "         1", 1UL);
	test_manual("{:>15}", "              1", 1UL);
	test_manual("{:>20}", "                   1", 1UL);

	test_manual("{:<1}", "1", 1UL);
	test_manual("{:<5}", "1    ", 1UL);
	test_manual("{:<10}", "1         ", 1UL);
	test_manual("{:<15}", "1              ", 1UL);
	test_manual("{:<20}", "1                   ", 1UL);

	test_manual("{:^1}", "1", 1UL);
	test_manual("{:^5}", "  1  ", 1UL);
	test_manual("{:^10}", "     1    ", 1UL);
	test_manual("{:^15}", "       1       ", 1UL);
	test_manual("{:^20}", "          1         ", 1UL);

	test_manual("{:か>1}", "1", 1UL);
	test_manual("{:か>5}", "かかかか1", 1UL);
	test_manual("{:か>10}", "かかかかかかかかか1", 1UL);
	test_manual("{:か>15}", "かかかかかかかかかかかかかか1", 1UL);
	test_manual("{:か>20}", "かかかかかかかかかかかかかかかかかかか1", 1UL);

	test_manual("{:🎅<1}", "1", 1UL);
	test_manual("{:🎅<5}", "1🎅🎅🎅🎅", 1UL);
	test_manual("{:🎅<10}", "1🎅🎅🎅🎅🎅🎅🎅🎅🎅", 1UL);
	test_manual("{:🎅<15}", "1🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 1UL);
	test_manual("{:🎅<20}", "1🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 1UL);

	test_manual("{:~^1}", "1", 1UL);
	test_manual("{:~^5}", "~~1~~", 1UL);
	test_manual("{:~^10}", "~~~~~1~~~~", 1UL);
	test_manual("{:~^15}", "~~~~~~~1~~~~~~~", 1UL);
	test_manual("{:~^20}", "~~~~~~~~~~1~~~~~~~~~", 1UL);

	// 64
	test_manual("{:>1}", "64", 64UL);
	test_manual("{:>5}", "   64", 64UL);
	test_manual("{:>10}", "        64", 64UL);
	test_manual("{:>15}", "             64", 64UL);
	test_manual("{:>20}", "                  64", 64UL);

	test_manual("{:<1}", "64", 64UL);
	test_manual("{:<5}", "64   ", 64UL);
	test_manual("{:<10}", "64        ", 64UL);
	test_manual("{:<15}", "64             ", 64UL);
	test_manual("{:<20}", "64                  ", 64UL);

	test_manual("{:^1}", "64", 64UL);
	test_manual("{:^5}", "  64 ", 64UL);
	test_manual("{:^10}", "    64    ", 64UL);
	test_manual("{:^15}", "       64      ", 64UL);
	test_manual("{:^20}", "         64         ", 64UL);

	test_manual("{:か>1}", "64", 64UL);
	test_manual("{:か>5}", "かかか64", 64UL);
	test_manual("{:か>10}", "かかかかかかかか64", 64UL);
	test_manual("{:か>15}", "かかかかかかかかかかかかか64", 64UL);
	test_manual("{:か>20}", "かかかかかかかかかかかかかかかかかか64", 64UL);

	test_manual("{:🎅<1}", "64", 64UL);
	test_manual("{:🎅<5}", "64🎅🎅🎅", 64UL);
	test_manual("{:🎅<10}", "64🎅🎅🎅🎅🎅🎅🎅🎅", 64UL);
	test_manual("{:🎅<15}", "64🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 64UL);
	test_manual("{:🎅<20}", "64🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 64UL);

	test_manual("{:~^1}", "64", 64UL);
	test_manual("{:~^5}", "~~64~", 64UL);
	test_manual("{:~^10}", "~~~~64~~~~", 64UL);
	test_manual("{:~^15}", "~~~~~~~64~~~~~~", 64UL);
	test_manual("{:~^20}", "~~~~~~~~~64~~~~~~~~~", 64UL);

	// 1000
	test_manual("{:>1}", "1000", 1000UL);
	test_manual("{:>5}", " 1000", 1000UL);
	test_manual("{:>10}", "      1000", 1000UL);
	test_manual("{:>15}", "           1000", 1000UL);
	test_manual("{:>20}", "                1000", 1000UL);

	test_manual("{:<1}", "1000", 1000UL);
	test_manual("{:<5}", "1000 ", 1000UL);
	test_manual("{:<10}", "1000      ", 1000UL);
	test_manual("{:<15}", "1000           ", 1000UL);
	test_manual("{:<20}", "1000                ", 1000UL);

	test_manual("{:^1}", "1000", 1000UL);
	test_manual("{:^5}", " 1000", 1000UL);
	test_manual("{:^10}", "   1000   ", 1000UL);
	test_manual("{:^15}", "      1000     ", 1000UL);
	test_manual("{:^20}", "        1000        ", 1000UL);

	test_manual("{:か>1}", "1000", 1000UL);
	test_manual("{:か>5}", "か1000", 1000UL);
	test_manual("{:か>10}", "かかかかかか1000", 1000UL);
	test_manual("{:か>15}", "かかかかかかかかかかか1000", 1000UL);
	test_manual("{:か>20}", "かかかかかかかかかかかかかかかか1000", 1000UL);

	test_manual("{:🎅<1}", "1000", 1000UL);
	test_manual("{:🎅<5}", "1000🎅", 1000UL);
	test_manual("{:🎅<10}", "1000🎅🎅🎅🎅🎅🎅", 1000UL);
	test_manual("{:🎅<15}", "1000🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 1000UL);
	test_manual("{:🎅<20}", "1000🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 1000UL);

	test_manual("{:~^1}", "1000", 1000UL);
	test_manual("{:~^5}", "~1000", 1000UL);
	test_manual("{:~^10}", "~~~1000~~~", 1000UL);
	test_manual("{:~^15}", "~~~~~~1000~~~~~", 1000UL);
	test_manual("{:~^20}", "~~~~~~~~1000~~~~~~~~", 1000UL);
}

Test(fmt_unsigne_long, precision_align) {
	for_each(unsigned long, x, 0, 1, 64, LONG_MAX, ULONG_MAX) {
		/* Width + precision, explicit right alignment */
		test_printf("{:>5.0}", "%5.0lu", x);
		test_printf("{:>5.0x}", "%5.0lx", x);
		test_printf("{:>5.0X}", "%5.0lX", x);
		test_printf("{:>5.0b}", "%5.0lb", x);
		test_printf("{:>5.0B}", "%5.0lB", x);
		test_printf("{:>10.3}", "%10.3lu", x);
		test_printf("{:>10.3x}", "%10.3lx", x);
		test_printf("{:>10.3X}", "%10.3lX", x);
		test_printf("{:>10.3b}", "%10.3lb", x);
		test_printf("{:>10.3B}", "%10.3lB", x);
		test_printf("{:>20.10}", "%20.10lu", x);
		test_printf("{:>20.10x}", "%20.10lx", x);
		test_printf("{:>20.10X}", "%20.10lX", x);
		test_printf("{:>20.10b}", "%20.10lb", x);
		test_printf("{:>20.10B}", "%20.10lB", x);
		test_printf("{:>5.10}", "%5.10lu", x);
		test_printf("{:>5.10x}", "%5.10lx", x);
		test_printf("{:>5.10X}", "%5.10lX", x);
		test_printf("{:>5.10b}", "%5.10lb", x);
		test_printf("{:>5.10B}", "%5.10lB", x);

		test_printf("{:<5.0}", "%-5.0lu", x);
		test_printf("{:<5.0x}", "%-5.0lx", x);
		test_printf("{:<5.0X}", "%-5.0lX", x);
		test_printf("{:<5.0b}", "%-5.0lb", x);
		test_printf("{:<5.0B}", "%-5.0lB", x);
		test_printf("{:<10.3}", "%-10.3lu", x);
		test_printf("{:<10.3x}", "%-10.3lx", x);
		test_printf("{:<10.3X}", "%-10.3lX", x);
		test_printf("{:<10.3b}", "%-10.3lb", x);
		test_printf("{:<10.3B}", "%-10.3lB", x);
		test_printf("{:<20.10}", "%-20.10lu", x);
		test_printf("{:<20.10x}", "%-20.10lx", x);
		test_printf("{:<20.10X}", "%-20.10lX", x);
		test_printf("{:<20.10b}", "%-20.10lb", x);
		test_printf("{:<20.10B}", "%-20.10lB", x);
		test_printf("{:<5.10}", "%-5.10lu", x);
		test_printf("{:<5.10x}", "%-5.10lx", x);
		test_printf("{:<5.10X}", "%-5.10lX", x);
		test_printf("{:<5.10b}", "%-5.10lb", x);
		test_printf("{:<5.10B}", "%-5.10lB", x);

		test_printf("{:>#5.0x}", "%#5.0lx", x);
		test_printf("{:>#5.0X}", "%#5.0lX", x);
		test_printf("{:>#5.0b}", "%#5.0lb", x);
		test_printf("{:>#5.0B}", "%#5.0lB", x);
		test_printf("{:>#10.3x}", "%#10.3lx", x);
		test_printf("{:>#10.3X}", "%#10.3lX", x);
		test_printf("{:>#10.3b}", "%#10.3lb", x);
		test_printf("{:>#10.3B}", "%#10.3lB", x);
		test_printf("{:>#20.10x}", "%#20.10lx", x);
		test_printf("{:>#20.10X}", "%#20.10lX", x);
		test_printf("{:>#20.10b}", "%#20.10lb", x);
		test_printf("{:>#20.10B}", "%#20.10lB", x);
		test_printf("{:>#5.10x}", "%#5.10lx", x);
		test_printf("{:>#5.10X}", "%#5.10lX", x);
		test_printf("{:>#5.10b}", "%#5.10lb", x);
		test_printf("{:>#5.10B}", "%#5.10lB", x);
		test_printf("{:<#5.0x}", "%-#5.0lx", x);
		test_printf("{:<#5.0X}", "%-#5.0lX", x);
		test_printf("{:<#5.0b}", "%-#5.0lb", x);
		test_printf("{:<#5.0B}", "%-#5.0lB", x);
		test_printf("{:<#10.3x}", "%-#10.3lx", x);
		test_printf("{:<#10.3X}", "%-#10.3lX", x);
		test_printf("{:<#10.3b}", "%-#10.3lb", x);
		test_printf("{:<#10.3B}", "%-#10.3lB", x);
		test_printf("{:<#20.10x}", "%-#20.10lx", x);
		test_printf("{:<#20.10X}", "%-#20.10lX", x);
		test_printf("{:<#20.10b}", "%-#20.10lb", x);
		test_printf("{:<#20.10B}", "%-#20.10lB", x);
		test_printf("{:<#5.10x}", "%-#5.10lx", x);
		test_printf("{:<#5.10X}", "%-#5.10lX", x);
		test_printf("{:<#5.10b}", "%-#5.10lb", x);
		test_printf("{:<#5.10B}", "%-#5.10lB", x);

		test_printf("{:5}", "%-5lu", x);
		test_printf("{:5.0}", "%-5.0lu", x);
		test_printf("{:10.3}", "%-10.3lu", x);
	}

	test_manual("{:^5.0}", "     ", 0UL);
	test_manual("{:^5.3}", " 000 ", 0UL);
	test_manual("{:^5.8}", "00000000", 0UL);
	test_manual("{:^10.0}", "          ", 0UL);
	test_manual("{:^10.3}", "    000   ", 0UL);
	test_manual("{:^10.8}", " 00000000 ", 0UL);
	test_manual("{:^15.0}", "               ", 0UL);
	test_manual("{:^15.3}", "      000      ", 0UL);
	test_manual("{:^15.8}", "    00000000   ", 0UL);
	test_manual("{:^5.0}", "  64 ", 64UL);
	test_manual("{:^5.3}", " 064 ", 64UL);
	test_manual("{:^5.8}", "00000064", 64UL);
	test_manual("{:^10.0}", "    64    ", 64UL);
	test_manual("{:^10.3}", "    064   ", 64UL);
	test_manual("{:^10.8}", " 00000064 ", 64UL);
	test_manual("{:^15.0}", "       64      ", 64UL);
	test_manual("{:^15.3}", "      064      ", 64UL);
	test_manual("{:^15.8}", "    00000064   ", 64UL);

	test_manual("{:か>5.0}", "かかかかか", 0UL);
	test_manual("{:か>5.3}", "かか000", 0UL);
	test_manual("{:か>10.0}", "かかかかかかかかかか", 0UL);
	test_manual("{:か>10.3}", "かかかかかかか000", 0UL);
	test_manual("{:か>5.0}", "かかか64", 64UL);
	test_manual("{:か>5.3}", "かか064", 64UL);
	test_manual("{:か>10.0}", "かかかかかかかか64", 64UL);
	test_manual("{:か>10.3}", "かかかかかかか064", 64UL);

	test_manual("{:🎅<5.0}", "🎅🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<5.3}", "000🎅🎅", 0UL);
	test_manual("{:🎅<10.0}", "🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<10.3}", "000🎅🎅🎅🎅🎅🎅🎅", 0UL);
	test_manual("{:🎅<5.0}", "64🎅🎅🎅", 64UL);
	test_manual("{:🎅<5.3}", "064🎅🎅", 64UL);
	test_manual("{:🎅<10.0}", "64🎅🎅🎅🎅🎅🎅🎅🎅", 64UL);
	test_manual("{:🎅<10.3}", "064🎅🎅🎅🎅🎅🎅🎅", 64UL);

	test_manual("{:~^5.0}", "~~~~~", 0UL);
	test_manual("{:~^5.3}", "~000~", 0UL);
	test_manual("{:~^10.0}", "~~~~~~~~~~", 0UL);
	test_manual("{:~^10.3}", "~~~~000~~~", 0UL);
	test_manual("{:~^5.0}", "~~64~", 64UL);
	test_manual("{:~^5.3}", "~064~", 64UL);
	test_manual("{:~^10.0}", "~~~~64~~~~", 64UL);
	test_manual("{:~^10.3}", "~~~~064~~~", 64UL);
}

Test(fmt_unsigned_long, dynamic_sizes) {
	for_each(unsigned long, x, 0, 1, 64, 123456, LONG_MAX, ULONG_MAX) {

		for_each(int, sizes, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
			test_printf("{1:>{0}}", "%*lu", sizes, x);
			test_printf("{1:>{0}x}", "%*lx", sizes, x);
			test_printf("{1:>{0}X}", "%*lX", sizes, x);
			test_printf("{1:>{0}b}", "%*lb", sizes, x);
			test_printf("{1:>{0}B}", "%*lB", sizes, x);

			test_printf("{1:.{0}}", "%.*lu", sizes, x);
			test_printf("{1:.{0}x}", "%.*lx", sizes, x);
			test_printf("{1:.{0}X}", "%.*lX", sizes, x);
			test_printf("{1:.{0}b}", "%.*lb", sizes, x);
			test_printf("{1:.{0}B}", "%.*lB", sizes, x);
		}

		for_each(int, prec, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
			for_each(int, width, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
				test_printf("{2:>{0}.{1}}", "%*.*lu", width, prec, x);
				test_printf("{2:<{0}.{1}}", "%-*.*lu", width, prec, x);

				test_printf("{2:>{0}.{1}x}", "%*.*lx", width, prec, x);
				test_printf("{2:<{0}.{1}x}", "%-*.*lx", width, prec, x);

				test_printf("{2:>{0}.{1}X}", "%*.*lX", width, prec, x);
				test_printf("{2:<{0}.{1}X}", "%-*.*lX", width, prec, x);

				test_printf("{2:>{0}.{1}b}", "%*.*lb", width, prec, x);
				test_printf("{2:<{0}.{1}b}", "%-*.*lb", width, prec, x);

				test_printf("{2:>{0}.{1}B}", "%*.*lB", width, prec, x);
				test_printf("{2:<{0}.{1}B}", "%-*.*lB", width, prec, x);
			}
		}
	}
}

Test(fmt_unsigned_long, bad_spec_conflicting_align_right, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:>05}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_conflicting_align_left, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:<05}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_conflicting_align_center, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:^05}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_precision_with_zero_pad, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:05.3}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_invalid_type, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:y}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_trailing_garbage, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:5xz}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_width_too_large, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:99999}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_precision_too_large, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:.99999}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_ref_missing_digit, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{0:{}}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_ref_out_of_range, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	/* Only one argument supplied; {1} inside the width reference
	 * points past it. */
	format(&out, "{0:{1}}", 1UL);
}

Test(fmt_unsigned_long, bad_spec_ref_missing_brace, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	/* Two args, so the out-of-range check passes and this isolates
	 * the "Expected `}' after number" check instead. */
	format(&out, "{0:{1x}}", 1UL, 2UL);
}
