#include "fmt.h"
#include <unistd.h>

int main()
{
	struct format_output out = format_output_fd(STDOUT_FILENO);
	format(&out, "Hello, World");
	format_output_destroy(&out);
	printf("%ld\n", 54l);
}
