#include "util.h"

Test(fmt_style, color) {
	test_manual("{fg#ff0000}", "\033[38;2;255;0;0m");
	test_manual("{fg#f00}", "\033[38;2;255;0;0m");
	test_manual("{fg#00ff00}", "\033[38;2;0;255;0m");
	test_manual("{fg#0f0}", "\033[38;2;0;255;0m");
	test_manual("{fg#0000ff}", "\033[38;2;0;0;255m");
	test_manual("{fg#00f}", "\033[38;2;0;0;255m");

	test_manual("{fg#000}", "\033[38;2;0;0;0m");
	test_manual("{fg#111}", "\033[38;2;17;17;17m");
	test_manual("{fg#222}", "\033[38;2;34;34;34m");
	test_manual("{fg#333}", "\033[38;2;51;51;51m");
	test_manual("{fg#444}", "\033[38;2;68;68;68m");
	test_manual("{fg#777}", "\033[38;2;119;119;119m");
	test_manual("{fg#fff}", "\033[38;2;255;255;255m");

	test_manual("{bg#ff0000}", "\033[48;2;255;0;0m");
	test_manual("{bg#f00}", "\033[48;2;255;0;0m");
	test_manual("{bg#00ff00}", "\033[48;2;0;255;0m");
	test_manual("{bg#0f0}", "\033[48;2;0;255;0m");
	test_manual("{bg#0000ff}", "\033[48;2;0;0;255m");
	test_manual("{bg#00f}", "\033[48;2;0;0;255m");

	test_manual("{bg#000}", "\033[48;2;0;0;0m");
	test_manual("{bg#111}", "\033[48;2;17;17;17m");
	test_manual("{bg#222}", "\033[48;2;34;34;34m");
	test_manual("{bg#333}", "\033[48;2;51;51;51m");
	test_manual("{bg#444}", "\033[48;2;68;68;68m");
	test_manual("{bg#777}", "\033[48;2;119;119;119m");
	test_manual("{bg#fff}", "\033[48;2;255;255;255m");

	test_manual("{fg#090909}", "\033[38;2;9;9;9m");
	test_manual("{fg#0a0a0a}", "\033[38;2;10;10;10m");
	
	test_manual("{fg#636363}", "\033[38;2;99;99;99m");
	test_manual("{fg#646464}", "\033[38;2;100;100;100m");

	test_manual("{bg#090909}", "\033[48;2;9;9;9m");
	test_manual("{bg#0a0a0a}", "\033[48;2;10;10;10m");

	test_manual("{bg#636363}", "\033[48;2;99;99;99m");
	test_manual("{bg#646464}", "\033[48;2;100;100;100m");
}
