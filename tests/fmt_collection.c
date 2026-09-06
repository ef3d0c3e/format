#include "util.h"

Test(fmt_collection, temp)
{
	struct format_output out = format_output_file(stdout);
	int arr[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17 };

	const char * stra[] = {
		"lorem",
		"ipsum",
		"dolor",
		"sit",
		"amet",
	};
	char *str = "Hello, World!";
	//format(&out, "{:[{1}]#({2}):{#x}}\n", FORMAT_ARRAY(str, format_fmt_signed_char), strlen(str), "");
	struct stat sb;
	format(&out, "{1:{0}}\n", 5, 6);
}
