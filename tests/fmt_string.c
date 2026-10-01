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

Test(fmt_string, display) {
	test_manual("{}", "Hello", "Hello");
	test_manual("{:s}", "Hello", "Hello");
	test_manual("{:?}", "Hello", "Hello");
	test_manual("{:x}", "Hello", "Hello");

	/* \t is always printed as-is */
	test_manual("{:s}", "\n\t\r", "\n\t\r");
	test_manual("{:?}", "\\n\t\\r", "\n\t\r");
	test_manual("{:x}", "0x0A\t0x0D", "\n\t\r");

	/* All escapable characters */
	test_manual("{:?}", "\\a\\b\\n\\v\\f\\r", "\a\b\n\v\f\r");
	test_manual("{:x}", "0x070x080x0A0x0B0x0C0x0D", "\a\b\n\v\f\r");

	/* Other non-printables */
	test_manual("{:s}", "\x01\x87", "\x01\x87");
	test_manual("{:?}", "\\x01\\x87", "\x01\x87");
	test_manual("{:x}", "0x010x87", "\x01\x87");

	/* Invalid UTF-8 is escaped like any other non-printable byte */
	test_manual("{:?}", "\\nT\\x87", "\nT\x87");
	test_manual("{:x}", "0x0AT0x87", "\nT\x87");

	/* Display types with quotes */
	test_manual("{:#''?}", "'\\n'", "\n");
	test_manual("{:#''x}", "'0x0A'", "\n");
}

Test(fmt_string, utf8) {
	/* Multi-byte code points count as one column */
	test_manual("|{:5}|", "|🎅    |", "🎅");
	test_manual("|{:<5}|", "|🎅    |", "🎅");
	test_manual("|{:>5}|", "|    🎅|", "🎅");
	test_manual("|{:^5}|", "|  🎅  |", "🎅");
	test_manual("|{:5}|", "|é🎅   |", "é🎅");
	test_manual("{:?}", "🎅", "🎅");
	test_manual("{:x}", "🎅", "🎅");
}

Test(fmt_string, precision_utf8) {
	/* Precision is in bytes and may split a code point */
	test_manual("{:.1}", "\xF0", "🎅");
	test_manual("{:.2}", "\xF0\x9F", "🎅");
	test_manual("{:.3}", "\xF0\x9F\x8E", "🎅");
	test_manual("{:.4}", "🎅", "🎅");
	test_manual("{:.1}", "\xC3", "é");
	test_manual("{:.2}", "é", "é");

	/* Truncated sequences are escaped when a display type asks for it */
	test_manual("{:.1?}", "\\xF0", "🎅");
	test_manual("{:.1x}", "0xF0", "🎅");
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
			sprintf(buf, "0x%02hhX", c);
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
			sprintf(buf, "\\x%02hhX", c);
		test_manual("{:?}", buf, str);
	}
}
