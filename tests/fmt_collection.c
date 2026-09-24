#include "util.h"

Test(fmt_collection, basic)
{
	const int int_arr[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, };
	const long long_arr[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, };
	const char* str_arr[16] = { "lorem", "ipsum", "dolor", "sit", "amet", "foo", "bar", "baz", "quz", "quz", "", "-", "{}", ".", "hello", "world" };
	for (size_t i = 0; i < 16; ++i)
	{
		char out[8192];
#define manual(start, end, fmt, fmt_sep, array) \
		do { \
			size_t pos = 0; \
			pos += (size_t)sprintf(out + pos, start); \
			for (size_t j = 0; j < i; ++j) \
			{ \
				if (j != 0) \
					pos += (size_t)sprintf(out + pos, fmt_sep, array[j]); \
				else \
					pos += (size_t)sprintf(out + pos, fmt, array[j]); \
			} \
			sprintf(out + pos, end); \
		} while (0)

		manual("{", "}", "%d", ", %d", int_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(int_arr), i);
		manual("{", "}", "%x", ", %x", int_arr);
		test_manual("{:[{1}]:{x}}", out, FORMAT_ARRAY(int_arr), i);

		manual("{", "}", "%ld", ", %ld", long_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(long_arr), i);
		manual("{", "}", "%lx", ", %lx", long_arr);
		test_manual("{:[{1}]:{x}}", out, FORMAT_ARRAY(long_arr), i);

		manual("{", "}", "%s", ", %s", str_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(str_arr, format_fmt_str), i);

		manual("[", "]", "%d", " %d", int_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(int_arr), i);
		manual("[", "]", "%x", " %x", int_arr);
		test_manual("{:[{1}]#[ ]:{x}}", out, FORMAT_ARRAY(int_arr), i);

		manual("[", "]", "%ld", " %ld", long_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(long_arr), i);
		manual("[", "]", "%lx", " %lx", long_arr);
		test_manual("{:[{1}]#[ ]:{x}}", out, FORMAT_ARRAY(long_arr), i);

		manual("[", "]", "%s", " %s", str_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(str_arr, format_fmt_str), i);

		manual("{<", ">}", "%d", "| |%d", int_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}", out, FORMAT_ARRAY(int_arr), i, "{<", "| |", ">}");
		manual("{<", ">}", "%x", "| |%x", int_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{x}}", out, FORMAT_ARRAY(int_arr), i, "{<", "| |", ">}");

		manual("{<", ">}", "%ld", "| |%ld", long_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}", out, FORMAT_ARRAY(long_arr), i, "{<", "| |", ">}");
		manual("{<", ">}", "%lx", "| |%lx", long_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{x}}", out, FORMAT_ARRAY(long_arr), i, "{<", "| |", ">}");

		manual("{<", ">}", "%s", "| |%s", str_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}", out, FORMAT_ARRAY(str_arr, format_fmt_str), i, "{<", "| |", ">}");
#undef manual
	}
}

