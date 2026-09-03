#include "util.h"

#define STRING_LIST "", \
	"a", \
	"ab", \
	"abc", \
	"abcd", \
	"abcde", \
	"abcdef", \
	"Hello", \
	"World", \
	"Hello, World!", \
	"lorem ipsum dolor sit amet", \
	"\x001", \
	"\x001\x002", \
	"\x001\x002\x003", \
	"\x001\x002\x003\x004", \
	"\x001\x002\x003\x004\x005", \
	"\x001\x002\x003\x004\x005\x006", \
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod " \
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim ven" \
	"iam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea com" \
	"modo consequat. Duis aute irure dolor in reprehenderit in voluptate veli" \
	"t esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat c" \
	"upidatat non proident, sunt in culpa qui officia deserunt mollit anim id" \
	" est laborum."


Test(fmt_string, format) {
	for_each(char*, str, STRING_LIST)
	{
		test_printf("{}", "%s", str);
		for (int precision = 0; precision < 128; ++precision)
		{
			char fmt1[1024];
			sprintf(fmt1, "{:.%d}", precision);
			char fmt2[1024];
			sprintf(fmt2, "%%.%ds", precision);

			test_printf(fmt1, fmt2, str);
			test_printf("{1:.{0}}", "%.*s", precision, str);
		}

		for (int width = 0; width < 128; ++width)
		{
			char fmt1[1024];
			sprintf(fmt1, "{:%d}", width);
			char fmt2[1024];
			sprintf(fmt2, "%%-%ds", width);

			test_printf(fmt1, fmt2, str);
			test_printf("{1:{0}}", "%-*s", width, str);
		}

		for (int precision = 0; precision < 128; ++precision)
		{
			for (int width = 0; width < 128; ++width)
			{
				{
					char fmt1[1024];
					sprintf(fmt1, "{:%d.%d}", width, precision);
					char fmt2[1024];
					sprintf(fmt2, "%%-%d.%ds", width, precision);

					test_printf(fmt1, fmt2, str);
					test_printf("{2:{0}.{1}}", "%-*.*s", width, precision, str);
				}
				{
					char fmt1[1024];
					sprintf(fmt1, "{:>%d.%d}", width, precision);
					char fmt2[1024];
					sprintf(fmt2, "%%%d.%ds", width, precision);

					test_printf(fmt1, fmt2, str);
					test_printf("{2:>{0}.{1}}", "%*.*s", width, precision, str);
				}
			}
		}
	}
}

Test(fmt_string, quotes) {
	struct pair {
		const char *left, *right;
	};

#define test(fmt_, printf_, quote1_, quote2_, string_) \
	do {  \
		char *quoted;  \
		asprintf(&quoted, "%s%s%s", quote1_, string_, quote2_);  \
		char *buf;  \
		asprintf(&buf, printf_, quoted);  \
		test_manual(fmt_, buf, string_);  \
		free(quoted);  \
		free(buf);  \
	} while(0)

	for_each(char*, str, STRING_LIST)
	{
		test("{:#''}", "%s", "'", "'", str);
		test("{:#ab}", "%s", "a", "b", str);
		test("{:#\"\"}", "%s", "\"", "\"", str);
		test("{:###}", "%s", "#", "#", str);
		test("{:#|~}", "%s", "|", "~", str);
		test("{:#🎅く}", "%s", "🎅", "く", str);

		for_each(int, width, 0, 1, 2, 3, 4, 5, 10, 15, 20, 100, 200)
		{
		
			for_each(struct pair, quotes,
					{ "'", "'" },
					{ "\"", "\"" },
					{ "a", "b" },
					{ "#", "#" },
					{ "|", "~" },
					{ "^", "-" },
					{ "%", "*" },
					{ " ", " " },
					{ "\n", "\t" })
			{
				char fmt1[1024];
				char fmt2[1024];

				sprintf(fmt1, "{:>%3$d#%1$s%2$s}", quotes.left, quotes.right, width);
				sprintf(fmt2, "%%%ds", width);
				test(fmt1, fmt2, quotes.left, quotes.right, str);

				sprintf(fmt1, "{:%3$d#%1$s%2$s}", quotes.left, quotes.right, width);
				sprintf(fmt2, "%%-%ds", width);
				test(fmt1, fmt2, quotes.left, quotes.right, str);
			}
		}

		for_each(struct pair, quotes,
				{ "'", "'" },
				{ "\"", "\"" },
				{ "a", "b" },
				{ "#", "#" },
				{ "|", "~" },
				{ "^", "-" },
				{ "%", "*" },
				{ " ", " " },
				{ "\n", "\t" })
		{
			char fmt1[1024];
			sprintf(fmt1, "{:#%s%s.0}", quotes.left, quotes.right);
			char out[1024];
			sprintf(out, "%s%s", quotes.left, quotes.right);
			const char* pout = out;
			test_manual(fmt1, pout, str);
		}

	}
#undef test
}


Test(fmt_string, temp) {
	struct format_output out = format_output_file(stdout);
	format(&out, "|{:{1}<20}|\n", "hello", " .");
	format(&out, "|{:{1}<20#{2}'}|\n", "hello", " .", "TEST");
}

