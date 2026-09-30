#include "util.h"

static int
readonly_fd(void)
{
	int fd = open("/dev/null", O_RDONLY);
	cr_assert(fd != -1, "Failed to open /dev/null");
	return fd;
}

Test(fmt_error, scalar)
{
	int fd = readonly_fd();
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushAlways);

	/* Writing to a read-only fd fails, the error must be reported */
	const int result = format(&out, "{}", 42);
	cr_assert_eq(result, -1);

	format_output_destroy(&out);
	close(fd);
}

Test(fmt_error, literal)
{
	int fd = readonly_fd();
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushAlways);

	const int result = format(&out, "hello");
	cr_assert_eq(result, -1);

	format_output_destroy(&out);
	close(fd);
}

Test(fmt_error, style)
{
	int fd = readonly_fd();
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushAlways);

	const int result = format(&out, "{fg#ff0000}");
	cr_assert_eq(result, -1);

	format_output_destroy(&out);
	close(fd);
}

Test(fmt_error, collection)
{
	const int arr[3] = {1, 2, 3};
	int fd = readonly_fd();
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushAlways);

	const int result = format(&out, "{:[3]:{}}", FORMAT_ARRAY(arr));
	cr_assert_eq(result, -1);

	format_output_destroy(&out);
	close(fd);
}

Test(fmt_error, flush)
{
	int fd = readonly_fd();
	struct format_output out = format_output_fd(fd); /* buffered */

	/* Buffered output does not fail until it is flushed */
	const int result = format(&out, "abc");
	cr_assert_eq(result, 0);
	cr_assert_eq(format_output_flush(&out), -1);

	format_output_destroy(&out);
	close(fd);
}