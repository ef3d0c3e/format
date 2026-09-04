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
	format(&out, "{:[{1}]#({2}):{^10#''}}\n", FORMAT_ARRAY(stra), 5, ", ");

	//format(&out, "|{:}|\n", (format_fmt_str, "abc"));
}
