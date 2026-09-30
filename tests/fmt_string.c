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

Test(fmt_string, fill) {
	test_manual("|{:{1}<20}|", "|hello. . . . . . . .|", "hello", " .");
	test_manual("|{:{1}<20}|", "|hello2 . . . . . . .|", "hello2", " .");

	test_manual("|{:{1}^20}|", "| . . . .hello. . . .|", "hello", " .");
	test_manual("|{:{1}^20}|", "| . . . hello2. . . .|", "hello2", " .");
	
	test_manual("|{:{1}>20}|", "| . . . . . . . hello|", "hello", " .");
	test_manual("|{:{1}>20}|", "| . . . . . . .hello2|", "hello2", " .");

	test_manual("|{:{1}<20}|", "|hello⨘ ⨘ ⨘ ⨘ ⨘ ⨘ ⨘ ⨘|", "hello", " ⨘");
	test_manual("|{:{1}<20}|", "|hello2 ⨘ ⨘ ⨘ ⨘ ⨘ ⨘ ⨘|", "hello2", " ⨘");

	test_manual("|{:{1}^20}|", "| ⨘ ⨘ ⨘ ⨘hello⨘ ⨘ ⨘ ⨘|", "hello", " ⨘");
	test_manual("|{:{1}^20}|", "| ⨘ ⨘ ⨘ hello2⨘ ⨘ ⨘ ⨘|", "hello2", " ⨘");
	
	test_manual("|{:{1}>20}|", "| ⨘ ⨘ ⨘ ⨘ ⨘ ⨘ ⨘ hello|", "hello", " ⨘");
	test_manual("|{:{1}>20}|", "| ⨘ ⨘ ⨘ ⨘ ⨘ ⨘ ⨘hello2|", "hello2", " ⨘");

	test_manual("|{:{1}<20}|", "|hello🎅 🎅 🎅 🎅 🎅 🎅 🎅 🎅|", "hello", " 🎅");
	test_manual("|{:{1}<20}|", "|hello2 🎅 🎅 🎅 🎅 🎅 🎅 🎅|", "hello2", " 🎅");

	test_manual("|{:{1}^20}|", "| 🎅 🎅 🎅 🎅hello🎅 🎅 🎅 🎅|", "hello", " 🎅");
	test_manual("|{:{1}^20}|", "| 🎅 🎅 🎅 hello2🎅 🎅 🎅 🎅|", "hello2", " 🎅");
	
	test_manual("|{:{1}>20}|", "| 🎅 🎅 🎅 🎅 🎅 🎅 🎅 hello|", "hello", " 🎅");
	test_manual("|{:{1}>20}|", "| 🎅 🎅 🎅 🎅 🎅 🎅 🎅hello2|", "hello2", " 🎅");
}

Test(fmt_string, escape) {
	test_manual("{:?}", "\\nT\\x87", "\nT\x87");
	test_manual("{:x}", "0x0aT0x87", "\nT\x87");

	for (size_t i = 1; i < 255; ++i)
	{
		const char c = (char)i;
		const char *str = (const char[2]){c, 0};

		char buf[256];
		
		sprintf(buf, "%c", c);
		test_manual("{}", buf, str);

		if (isprint(c) || c == '\t')
			sprintf(buf, "%c", c);
		else
			sprintf(buf, "0x%02hhx", c);
		test_manual("{:x}", buf, str);

		if (isprint(c) || c == '\t')
			sprintf(buf, "%c", c);
		else if (strchr("\n\r\v\f\a\b", c))
		{
			char escape = 0;
			switch (c)
			{
				case '\a':
					escape = 'a';
					break;
				case '\b':
					escape = 'b';
					break;
				case '\n':
					escape = 'n';
					break;
				case '\v':
					escape = 'v';
					break;
				case '\f':
					escape = 'f';
					break;
				case '\r':
					escape = 'r';
					break;
				default:
					break;
			}
			sprintf(buf, "\\%c", escape);
		}
		else
			sprintf(buf, "\\x%02hhx", c);
		test_manual("{:?}", buf, str);
	}
}
