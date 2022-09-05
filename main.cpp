#include "header.h"
int main(int argc, char **argv)
{
	char *fileName;
	if (argc>1)
		fileName=argv[1];
	else
		fileName=nullptr;
	vim_r editor(fileName);
	editor.run();
	return 0;
}
