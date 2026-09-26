#include "util.h"

Test(fmt_char, types) {
	char buf[64];

	// :c
	for (size_t i = 1; i < 256; ++i)
	{
		memset(buf, 0, sizeof(buf));
		const char c = (char)i;
		buf[0] = c;
		test_manual("{}", buf, c);
		test_manual("{:c}", buf, c);
	}

	// :x
	for (size_t i = 0; i < 256; ++i)
	{
		memset(buf, 0, sizeof(buf));
		const char c = (char)i;
		if (isprint(c) || c == '\t')
			buf[0] = c;
		else
		{
			buf[0] = '0';
			buf[1] = 'x';
			buf[2] = "0123456789abcdef"[i / 16];
			buf[3] = "0123456789abcdef"[i % 16];
		}
		test_manual("{:x}", buf, c);
	}

	// :?
	for (size_t i = 0; i < 256; ++i)
	{
		memset(buf, 0, sizeof(buf));
		const char c = (char)i;
		if (isprint(c) || c == '\t')
			buf[0] = c;
		else if (c != 0 && strchr("\a\b\n\v\f\r", c))
		{
			buf[0] = '\\';
			switch (c) {
				case '\a':
					buf[1] = 'a';
					break;
				case '\b':
					buf[1] = 'b';
					break;
				case '\n':
					buf[1] = 'n';
					break;
				case '\v':
					buf[1] = 'v';
					break;
				case '\f':
					buf[1] = 'f';
					break;
				case '\r':
					buf[1] = 'r';
					break;
				default:
					format_unreachable();
			}
		}
		else
		{
			buf[0] = '\\';
			buf[1] = 'x';
			buf[2] = "0123456789abcdef"[i / 16];
			buf[3] = "0123456789abcdef"[i % 16];
		}
		test_manual("{:?}", buf, c);
	}
}

Test(fmt_char, quotes) {
	struct pair {
		const char *left, *right;
	};

#define test(fmt_, printf_, quote1_, quote2_, char_) \
	do {  \
		char *quoted;  \
		asprintf(&quoted, "%s%c%s", quote1_, char_, quote2_);  \
		char *buf;  \
		asprintf(&buf, printf_, quoted);  \
		test_manual(fmt_, buf, char_);  \
		free(quoted);  \
		free(buf);  \
	} while(0)

	// :c
	for (size_t i = 1; i < 256; ++i)
	{
		const char c = (char)i;
		test("{:#''}", "%s", "'", "'", c);
		test("{:#ab}", "%s", "a", "b", c);
		test("{:#\"\"}", "%s", "\"", "\"", c);
		test("{:###}", "%s", "#", "#", c);
		test("{:#|~}", "%s", "|", "~", c);
		test("{:#🎅く}", "%s", "🎅", "く", c);

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
				test(fmt1, fmt2, quotes.left, quotes.right, c);

				sprintf(fmt1, "{:%3$d#%1$s%2$s}", quotes.left, quotes.right, width);
				sprintf(fmt2, "%%-%ds", width);
				test(fmt1, fmt2, quotes.left, quotes.right, c);
			}
		}
	}
#undef test
}
