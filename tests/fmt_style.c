#include "util.h"

Test(fmt_style, colors)
{
	test_manual("{fg#ff0000}", "\033[38;2;255;0;0m");
	test_manual("{fg#00ff00}", "\033[38;2;0;255;0m");
	test_manual("{fg#0000ff}", "\033[38;2;0;0;255m");
	test_manual("{bg#ff0000}", "\033[48;2;255;0;0m");
	test_manual("{bg#00ff00}", "\033[48;2;0;255;0m");
	test_manual("{bg#0000ff}", "\033[48;2;0;0;255m");
	test_manual("{fg#ff0000 bg#00ff00}", "\033[38;2;255;0;0m\033[48;2;0;255;0m");
	test_manual("{bg#00ff00 fg#ff0000}", "\033[38;2;255;0;0m\033[48;2;0;255;0m");

	/* Zero components */
	test_manual("{fg#000000}", "\033[38;2;0;0;0m");

	/* Components equal to 10 and 100 */
	test_manual("{fg#0a0a0a}", "\033[38;2;10;10;10m");
	test_manual("{fg#646464}", "\033[38;2;100;100;100m");
	test_manual("{fg#0a6401}", "\033[38;2;10;100;1m");

	/* 9 and 99 should emit 1 and 2 digits respectively */
	test_manual("{bg#090909}", "\033[48;2;9;9;9m");
	test_manual("{bg#0a0a0a}", "\033[48;2;10;10;10m");

	test_manual("{bg#636363}", "\033[48;2;99;99;99m");

	/* 3-digit shorthand expands each digit */
	test_manual("{fg#abc}", "\033[38;2;170;187;204m");
	test_manual("{bg#f0a}", "\033[48;2;255;0;170m");
	test_manual("{bg#000}", "\033[48;2;0;0;0m");
	test_manual("{bg#fff}", "\033[48;2;255;255;255m");

	/* Color from argument */
	test_manual("{fg{1}}", "\033[38;2;18;52;86m", 0x123456);
	test_manual("{bg{1}}", "\033[48;2;255;255;255m", 0xFFFFFF);

	/* Same color is not emitted twice in a row */
	test_manual("{fg#ff0000}{fg#ff0000}", "\033[38;2;255;0;0m");
}

Test(fmt_style, styles)
{
	test_manual("{/b}", "\033[1m");
	test_manual("{/i}", "\033[3m");
	test_manual("{/u}", "\033[4m");
	test_manual("{/c}", "\033[9m");
	test_manual("{/bciu}", "\033[1m\033[3m\033[4m\033[9m");
	test_manual("{/0}", "\033[0m");

	/* Toggling off resets everything, then restores the remaining style */
	test_manual("{/b}{/b}", "\033[1m\033[0m");
	test_manual("{/bi}{/b}", "\033[1m\033[3m\033[0m\033[3m");
	test_manual("{/biu}{/i}", "\033[1m\033[3m\033[4m\033[0m\033[1m\033[4m");

	/* Reset clears colors too */
	test_manual("{fg#ff0000 /b}{/0}", "\033[38;2;255;0;0m\033[1m\033[0m");
	test_manual("{fg#ff0000}{/b}", "\033[38;2;255;0;0m\033[1m");
}

Test(fmt_style, mixed)
{
	test_manual("{fg#ff0000}Hello {/b}World {/0}!", "\033[38;2;255;0;0mHello \033[1mWorld \033[0m!");
	test_manual("{fg#ff0000\t/b}", "\033[38;2;255;0;0m\033[1m");
	test_manual("{fg#ff0000   bg#00ff00\t\t/u}", "\033[38;2;255;0;0m\033[48;2;0;255;0m\033[4m");
}

Test(fmt_style, bad_color_digits, .signal = SIGABRT)
{
	test_manual("{fg#12345}", "");
}

Test(fmt_style, bad_color_extra_digits, .signal = SIGABRT)
{
	test_manual("{fg#1234567}", "");
}

Test(fmt_style, bad_style, .signal = SIGABRT)
{
	test_manual("{xx}", "");
}

Test(fmt_style, bad_color_value, .signal = SIGABRT)
{
	test_manual("{fg{1}}", "", 0x1000000);
}
>>>>>>> f7314f5 (test: cover style and colors)
