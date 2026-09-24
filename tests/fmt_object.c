#include "util.h"

struct Bar
{
	int x;
	int y;
};

FORMAT_OBJ(struct Bar,
           format_bar,
           ((format_fmt_int, x), ""),
           ((format_fmt_int, y), ""))

struct Foo
{
	int val;
	const char* str;
	long x;
	int arr[5];
	size_t len;
	struct Bar bar;
};

FORMAT_OBJ(struct Foo,
           format_foo,
           ((format_fmt_int, val), "x"),
           (val, "b"),
           (FORMAT_ARRAY(arr), "[{1}]:{}", (FORMAT_OBJ_STRUCT->len)),
		   (FORMAT_SUBOBJ(format_bar, bar), "2"))

Test(fmt_object, temp)
{
	//struct format_output out = format_output_file(stdout);
	struct Foo foo = {
		.val = 0x64,
		.str = "Hello,\n World!",
		.x = 123456,
		.arr = {1,2,3,4,5},
		.len = 4,
		.bar = {6,7},
	};
	//format(&out, "{:1}\n", (format_foo, &foo));
	test_manual("{:1}", "struct Foo {\n    val = 64,\n    val = 1100100,\n    arr = {1, 2, 3, 4},\n    bar = struct Bar {\n        x = 6,\n        y = 7,\n    },\n}", (format_foo, &foo));
}
