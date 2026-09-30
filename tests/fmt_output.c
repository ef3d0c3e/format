#include "util.h"

Test(fmt_output, escaped_braces)
{
	test_manual("{{}}", "{}");
	test_manual("{{", "{");
	test_manual("}}", "}");
	test_manual("a{{b}}c", "a{b}c");
	test_manual("{{}}{{}}", "{}{}");
	test_manual("{{{0}}}", "{1}", 1);
}

Test(fmt_output, argument_reorder)
{
	test_manual("{1} {0}", "2 1", 1, 2);
	test_manual("{0} {1} {0}", "1 2 1", 1, 2);
	test_manual("{2}{1}{0}", "cba", "a", "b", "c");
}

Test(fmt_output, buf_append)
{
	struct format_output out = format_output_buf();

	const int result1 = format(&out, "Hello, ");
	cr_assert_eq(result1, 0);
	cr_assert_eq(out.size, 7);

	const int result2 = format(&out, "{}!", "World");
	cr_assert_eq(result2, 0);
	cr_assert_eq(out.size, 13);
	cr_assert(memcmp(out.data, "Hello, World!", 13) == 0);

	/* Flushing a memory output is a no-op */
	cr_assert_eq(format_output_flush(&out), 0);
	cr_assert_eq(out.size, 13);

	format_output_destroy(&out);
}

Test(fmt_output, buf_growth)
{
	char* big = malloc(5001);
	memset(big, 'z', 5000);
	big[5000] = '\0';

	struct format_output out = format_output_buf();
	const int result = format(&out, "{}", big);
	cr_assert_eq(result, 0);
	cr_assert_eq(out.size, 5000);
	cr_assert(memcmp(out.data, big, 5000) == 0);

	format_output_destroy(&out);
	free(big);
}

Test(fmt_output, none)
{
	struct format_output out = format_output_none();
	const int result = format(&out, "|{:>10}|", "hi");
	cr_assert_eq(result, 0);
	cr_assert_eq(out.size, 12);
	cr_assert(out.data == NULL);

	format_output_destroy(&out);
}