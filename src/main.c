#include "fmt.h"
#include <unistd.h>

int main()
{
	struct format_output out = format_output_fd(STDOUT_FILENO);
	format(&out, "Hello, World |{fg#ff0000}{0:'^{1}}{/0}|", 254l, 20l, 0xff0000l);
	format_output_destroy(&out);
	//printf("%ld\n", 54l);
}
