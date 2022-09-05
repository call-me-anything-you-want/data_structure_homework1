#include "vim_r.h"
int main(int argc, char **argv)
{
	if (argc>1)
		vim_r editor(argv[1]);
	else
		vim_r editor(nullptr);
	editor.run();
	return 0;
}
