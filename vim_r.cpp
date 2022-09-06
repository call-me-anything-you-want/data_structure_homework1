#include "header.h"
#include<fstream>
#include<iostream>
#include<string>
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
			if (ch>0)
				this->takeAction((int)ch);
			else
			{
				// del, up, down, left, right, f1-f12
				ch=_getch();
				this->takeAction((int)ch+256);
			}
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

void vim_r::takeAction(int ch)
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

void vim_r::takeActionNormal(int ch)
{
	if (ch=='j')
		this->cp.moveDown();
	else if (ch=='k')
		this->cp.moveUp();
	else if (ch=='l')
		this->cp.moveRight();
	else if (ch=='h')
		this->cp.moveLeft();
	else if (ch=='i')
	{
		this->m=INSERT;
		if (this->ft==nullptr)
		{
			this->ft=new fileContent();
			this->cp.linePos=this->ft;
			this->cp.charPos=0;
		}
		int currentLineLen=size(this->cp.linePos->line);
		this->cp.charPos=this->cp.charPos>=currentLineLen ? currentLineLen-1 : this->cp.charPos;
	}
	else if (ch=='a')
	{
		this->m=INSERT;
		if (this->ft==nullptr)
		{
			this->ft=new fileContent();
			this->cp.linePos=this->ft;
			this->cp.charPos=0;
		}
		else
		{
			int currentLineLen=size(this->cp.linePos->line);
			if (currentLineLen==0)
				this->cp.charPos=0;
			else
				this->cp.charPos=this->cp.charPos>currentLineLen ? currentLineLen : this->cp.charPos+1;
		}
	}
	else if (ch==':')
		this->m=EX;
}

void vim_r::takeActionInsert(int ch)
{
	if (ch=='\n')
	{
		// create a new line
		int currentLineLen=size(this->cp.linePos->line);
		int currentCol=this->cp.charPos>currentLineLen ? currentLineLen : this->cp.charPos;
		string newline;
		if (currentCol==currentLineLen)
			newline="";
		else
		{
			newline=this->cp.linePos->line.substr(currentCol);
			this->cp.linePos->line=this->cp.linePos->line.substr(0, currentCol);
		}
		fileContent *temp=new fileContent(newline);
		temp->next=this->cp.linePos->next;
		if (temp->next!=nullptr)
			temp->next->prev=temp;
		temp->prev=this->cp.linePos;
		temp->prev->next=temp;

		// move cursor
		this->cp.linePos=temp;
		this->cp.charPos=0;
	}
	else if (ch=='\b')
	{
		// delete a character
		int currentLineLen=size(this->cp.linePos->line);
		int currentCol=this->cp.charPos>currentLineLen ? currentLineLen : this->cp.charPos;
		if (currentCol!=0) // if the cursor is not at the front of this line, just delete a char
		{
			this->cp.linePos->line.erase(currentCol-1, 1);
			this->cp.charPos=currentCol-1;
		}
		else
		{
			if (this->cp.linePos->prev!=nullptr) // if the cursor is not at the front of the whole file
			{
				this->cp.charPos=size(this->cp.linePos->prev->line);
				this->cp.linePos->prev->line+=this->cp.linePos->line;
				fileContent *temp=this->cp.linePos;
				this->cp.linePos->prev->next=this->cp.linePos->next;
				if (this->cp.linePos->next!=nullptr)
					this->cp.linePos->next->prev=this->cp.linePos->prev;
				this->cp.linePos=this->cp.linePos->prev;
				delete temp;
			}
		}
	}
	else if (ch==83+256)
	{
		// del key
	}
	else if (ch==72+256 || ch==80+256 || ch==75+256 || ch==77+256)
	{
		// up, down, left, right
		// do nothing for now
	}
	else if ((ch>=59+256 && ch<=69+256) || (ch==-122+256))
	{
		// f1, f2, ..., f12
		// do nothing for now
	}
	else
	{
		// just insert char(ch) would be fine.
	}
}
