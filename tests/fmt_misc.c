#include "util.h"

Test(fmt_misc, write_error) {
	// stdin
	{
		struct format_output __attribute__((cleanup(format_output_destroy))) out = format_output_file(stdin);
		{
			int result = format(&out, "{}", 42);
			cr_assert(result != 0);
		}

		{
			int result = format_output_write(&out, "foo", 3);
			cr_assert(result != 0);
		}
	}
	
	// fd 0
	{
		struct format_output __attribute__((cleanup(format_output_destroy))) out = format_output_fd(0);
		format_output_set_flush(&out, kFormatFlushAlways);
		{
			int result = format(&out, "{}", 42);
			cr_assert(result != 0);
		}

		{
			int result = format_output_write(&out, "foo", 3);
			cr_assert(result != 0);
		}
	}
}
