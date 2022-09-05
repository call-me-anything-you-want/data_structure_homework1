#include "header.h"
#include<fstream>
#include<iostream>
#include<conio.h>
using namespace std;
vim_r::vim_r(char *filename)
{
	this->m=NORMAL;
	if (filename!=nullptr)
	{
		ifstream fin;
		fin.open(filename, ios::in);
		if (fin.fail())
		{
			cout << "can't open file " << filename << ".\n";
			exit(0);
		}
		// put the file content into this->ft;
	}
	else
	{
		// initialize this->ft as empty
	}
}

void vim_r::run()
{
	while(1)
	{
		if (_kbhit())
		{
			char ch=_getch();
			this->takeAction(ch);
		}
		// display current file content
	}
}
