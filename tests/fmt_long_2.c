#include "util.h"


Test(fmt_long, precision_align) {
	for_each(long, x, -1, 0, 1, 64, LONG_MAX, LONG_MIN) {
		/* Width + precision, explicit right alignment */
		test_printf("{:>5.0}", "%5.0ld", x);
		test_printf("{:>5.0x}", "%5.0lx", x);
		test_printf("{:>5.0X}", "%5.0lX", x);
		test_printf("{:>5.0b}", "%5.0lb", x);
		test_printf("{:>5.0B}", "%5.0lB", x);
		test_printf("{:>10.3}", "%10.3ld", x);
		test_printf("{:>10.3x}", "%10.3lx", x);
		test_printf("{:>10.3X}", "%10.3lX", x);
		test_printf("{:>10.3b}", "%10.3lb", x);
		test_printf("{:>10.3B}", "%10.3lB", x);
		test_printf("{:>20.10}", "%20.10ld", x);
		test_printf("{:>20.10x}", "%20.10lx", x);
		test_printf("{:>20.10X}", "%20.10lX", x);
		test_printf("{:>20.10b}", "%20.10lb", x);
		test_printf("{:>20.10B}", "%20.10lB", x);
		test_printf("{:>5.10}", "%5.10ld", x);
		test_printf("{:>5.10x}", "%5.10lx", x);
		test_printf("{:>5.10X}", "%5.10lX", x);
		test_printf("{:>5.10b}", "%5.10lb", x);
		test_printf("{:>5.10B}", "%5.10lB", x);

		test_printf("{:<5.0}", "%-5.0ld", x);
		test_printf("{:<5.0x}", "%-5.0lx", x);
		test_printf("{:<5.0X}", "%-5.0lX", x);
		test_printf("{:<5.0b}", "%-5.0lb", x);
		test_printf("{:<5.0B}", "%-5.0lB", x);
		test_printf("{:<10.3}", "%-10.3ld", x);
		test_printf("{:<10.3x}", "%-10.3lx", x);
		test_printf("{:<10.3X}", "%-10.3lX", x);
		test_printf("{:<10.3b}", "%-10.3lb", x);
		test_printf("{:<10.3B}", "%-10.3lB", x);
		test_printf("{:<20.10}", "%-20.10ld", x);
		test_printf("{:<20.10x}", "%-20.10lx", x);
		test_printf("{:<20.10X}", "%-20.10lX", x);
		test_printf("{:<20.10b}", "%-20.10lb", x);
		test_printf("{:<20.10B}", "%-20.10lB", x);
		test_printf("{:<5.10}", "%-5.10ld", x);
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

		test_printf("{:>+10.0}", "%+10.0ld", x);
		test_printf("{:<+10.0}", "%- +10.0ld", x);
		test_printf("{:>+10.3}", "%+10.3ld", x);
		test_printf("{:<+10.3}", "%- +10.3ld", x);
		test_printf("{:>+10.10}", "%+10.10ld", x);
		test_printf("{:<+10.10}", "%- +10.10ld", x);
		test_printf("{:> 10.0}", "% 10.0ld", x);
		test_printf("{:< 10.0}", "%- 10.0ld", x);
		test_printf("{:> 10.3}", "% 10.3ld", x);
		test_printf("{:< 10.3}", "%- 10.3ld", x);
		test_printf("{:> 10.10}", "% 10.10ld", x);
		test_printf("{:< 10.10}", "%- 10.10ld", x);

		test_printf("{:5}", "%-5ld", x);
		test_printf("{:5.0}", "%-5.0ld", x);
		test_printf("{:10.3}", "%-10.3ld", x);
	}

	test_manual("{:^5.0}", "  -1 ", -1L);
	test_manual("{:^5.3}", " -001", -1L);
	test_manual("{:^5.8}", "-00000001", -1L);
	test_manual("{:^10.0}", "    -1    ", -1L);
	test_manual("{:^10.3}", "   -001   ", -1L);
	test_manual("{:^10.8}", " -00000001", -1L);
	test_manual("{:^15.0}", "       -1      ", -1L);
	test_manual("{:^15.3}", "      -001     ", -1L);
	test_manual("{:^15.8}", "   -00000001   ", -1L);
	test_manual("{:^5.0}", "     ", 0L);
	test_manual("{:^5.3}", " 000 ", 0L);
	test_manual("{:^5.8}", "00000000", 0L);
	test_manual("{:^10.0}", "          ", 0L);
	test_manual("{:^10.3}", "    000   ", 0L);
	test_manual("{:^10.8}", " 00000000 ", 0L);
	test_manual("{:^15.0}", "               ", 0L);
	test_manual("{:^15.3}", "      000      ", 0L);
	test_manual("{:^15.8}", "    00000000   ", 0L);
	test_manual("{:^5.0}", "  64 ", 64L);
	test_manual("{:^5.3}", " 064 ", 64L);
	test_manual("{:^5.8}", "00000064", 64L);
	test_manual("{:^10.0}", "    64    ", 64L);
	test_manual("{:^10.3}", "    064   ", 64L);
	test_manual("{:^10.8}", " 00000064 ", 64L);
	test_manual("{:^15.0}", "       64      ", 64L);
	test_manual("{:^15.3}", "      064      ", 64L);
	test_manual("{:^15.8}", "    00000064   ", 64L);
	test_manual("{:^5.0}", "-123456", -123456L);
	test_manual("{:^5.3}", "-123456", -123456L);
	test_manual("{:^5.8}", "-00123456", -123456L);
	test_manual("{:^10.0}", "  -123456 ", -123456L);
	test_manual("{:^10.3}", "  -123456 ", -123456L);
	test_manual("{:^10.8}", " -00123456", -123456L);
	test_manual("{:^15.0}", "    -123456    ", -123456L);
	test_manual("{:^15.3}", "    -123456    ", -123456L);
	test_manual("{:^15.8}", "   -00123456   ", -123456L);

	test_manual("{:か>5.0}", "かかか-1", -1L);
	test_manual("{:か>5.3}", "か-001", -1L);
	test_manual("{:か>10.0}", "かかかかかかかか-1", -1L);
	test_manual("{:か>10.3}", "かかかかかか-001", -1L);
	test_manual("{:か>5.0}", "かかかかか", 0L);
	test_manual("{:か>5.3}", "かか000", 0L);
	test_manual("{:か>10.0}", "かかかかかかかかかか", 0L);
	test_manual("{:か>10.3}", "かかかかかかか000", 0L);
	test_manual("{:か>5.0}", "かかか64", 64L);
	test_manual("{:か>5.3}", "かか064", 64L);
	test_manual("{:か>10.0}", "かかかかかかかか64", 64L);
	test_manual("{:か>10.3}", "かかかかかかか064", 64L);

	test_manual("{:🎅<5.0}", "-1🎅🎅🎅", -1L);
	test_manual("{:🎅<5.3}", "-001🎅", -1L);
	test_manual("{:🎅<10.0}", "-1🎅🎅🎅🎅🎅🎅🎅🎅", -1L);
	test_manual("{:🎅<10.3}", "-001🎅🎅🎅🎅🎅🎅", -1L);
	test_manual("{:🎅<5.0}", "🎅🎅🎅🎅🎅", 0L);
	test_manual("{:🎅<5.3}", "000🎅🎅", 0L);
	test_manual("{:🎅<10.0}", "🎅🎅🎅🎅🎅🎅🎅🎅🎅🎅", 0L);
	test_manual("{:🎅<10.3}", "000🎅🎅🎅🎅🎅🎅🎅", 0L);
	test_manual("{:🎅<5.0}", "64🎅🎅🎅", 64L);
	test_manual("{:🎅<5.3}", "064🎅🎅", 64L);
	test_manual("{:🎅<10.0}", "64🎅🎅🎅🎅🎅🎅🎅🎅", 64L);
	test_manual("{:🎅<10.3}", "064🎅🎅🎅🎅🎅🎅🎅", 64L);

	test_manual("{:~^5.0}", "~~-1~", -1L);
	test_manual("{:~^5.3}", "~-001", -1L);
	test_manual("{:~^10.0}", "~~~~-1~~~~", -1L);
	test_manual("{:~^10.3}", "~~~-001~~~", -1L);
	test_manual("{:~^5.0}", "~~~~~", 0L);
	test_manual("{:~^5.3}", "~000~", 0L);
	test_manual("{:~^10.0}", "~~~~~~~~~~", 0L);
	test_manual("{:~^10.3}", "~~~~000~~~", 0L);
	test_manual("{:~^5.0}", "~~64~", 64L);
	test_manual("{:~^5.3}", "~064~", 64L);
	test_manual("{:~^10.0}", "~~~~64~~~~", 64L);
	test_manual("{:~^10.3}", "~~~~064~~~", 64L);
}

Test(fmt_long, dynamic_sizes) {
	for_each(long, x, -123456, -1, 0, 1, 64, LONG_MAX, LONG_MIN) {

		for_each(int, sizes, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
			test_printf("{1:>{0}}", "%*ld", sizes, x);
			test_printf("{1:>{0}x}", "%*lx", sizes, x);
			test_printf("{1:>{0}X}", "%*lX", sizes, x);
			test_printf("{1:>{0}b}", "%*lb", sizes, x);
			test_printf("{1:>{0}B}", "%*lB", sizes, x);

			test_printf("{1:.{0}}", "%.*ld", sizes, x);
			test_printf("{1:.{0}x}", "%.*lx", sizes, x);
			test_printf("{1:.{0}X}", "%.*lX", sizes, x);
			test_printf("{1:.{0}b}", "%.*lb", sizes, x);
			test_printf("{1:.{0}B}", "%.*lB", sizes, x);
		}

		for_each(int, prec, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
			for_each(int, width, 0, 1, 2, 3, 5, 10, 15, 20, 50, 100) {
				test_printf("{2:>{0}.{1}}", "%*.*ld", width, prec, x);
				test_printf("{2:<{0}.{1}}", "%-*.*ld", width, prec, x);

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

Test(fmt_long, bad_spec_conflicting_align_right, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:>05}", 1L);
}

Test(fmt_long, bad_spec_conflicting_align_left, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:<05}", 1L);
}

Test(fmt_long, bad_spec_conflicting_align_center, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:^05}", 1L);
}

Test(fmt_long, bad_spec_precision_with_zero_pad, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:05.3}", 1L);
}

Test(fmt_long, bad_spec_invalid_type, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:y}", 1L);
}

Test(fmt_long, bad_spec_trailing_garbage, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:5xz}", 1L);
}

Test(fmt_long, bad_spec_width_too_large, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:99999}", 1L);
}

Test(fmt_long, bad_spec_precision_too_large, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{:.99999}", 1L);
}

Test(fmt_long, bad_spec_ref_missing_digit, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{0:{}}", 1L);
}

Test(fmt_long, bad_spec_ref_out_of_range, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	/* Only one argument supplied; {1} inside the width reference
	 * points past it */
	format(&out, "{0:{1}}", 1L);
}

Test(fmt_long, bad_spec_ref_missing_brace, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	/* Two args, so the out-of-range check passes and this isolates
	 * the "Expected `}' after number" check instead */
	format(&out, "{0:{1x}}", 1L, 2L);
}

Test(fmt_long, bad_spec_invalid_display_type, .signal = SIGABRT) {
	struct format_output out = format_output_buf();
	format(&out, "{0:c}", 1L);
}
