#include "header.h"
#include<fstream>
#include<iostream>
#include<conio.h>
using namespace std;
vim_r::vim_r(char *filename) : cp()
{
	this->ft=nullptr;
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
		string temp;
		while (getline(fin, temp))
		{
			if (this->ft==nullptr) // empty file content
			{
				this->ft=new fileContent(temp);
				this->cp.linePos=this->ft;
			}
			else
			{
				fileContent *newline=new fileContent(temp);
				newline->prev=this->cp.linePos;
				this->cp.linePos->next=newline;
				this->cp.linePos=newline;
			}
		}
		this->cp.linePos=this->ft;
	}
	else
	{
		this->ft=nullptr;
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
		// use 2 buffers to solve the current flash
		system("cls");
		fileContent *temp=this->ft;
		while (temp!=nullptr)
		{
			cout << temp->line << "\n";
			temp=temp->next;
		}
	}
}

void vim_r::takeAction(char ch)
{
	if (this->m==NORMAL)
		takeActionNormal(ch);
	else if (this->m==INSERT)
		takeActionInsert(ch);
	else if (this->m==EX)
		takeActionEx(ch);
	else if (this->m==VISUAL)
		takeActionVisual(ch);
}

void vim_r::takeActionNormal(char ch)
{}
