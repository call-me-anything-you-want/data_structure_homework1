#include "header.h"
#include<fstream>
#include<iostream>
#include<string>
#include<conio.h>
using namespace std;
vim_r::vim_r(char *filename) : cp()
{
	if (filename==nullptr)
		this->filename="";
	else
	{
		this->filename=string(filename);
		int temp=size(this->filename);
		while (temp>=0 && this->filename[temp]!='/' && this->filename[temp]!='\\')
			temp--;
		this->filename=this->filename.substr(temp+1);
	}
	this->ft=nullptr;
	this->m=NORMAL;
	this->message="";
	if (filename!=nullptr)
	{
		ifstream fin;
		fin.open(filename, ios::in);
		if (fin.fail())
		{
			this->message="can't open file "+string(filename)+".";
			this->filename="";
		}
		else
		{
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
	if (ch<=127)
	{
		if ((char)ch=='j')
			this->cp.moveDown();
		else if ((char)ch=='k')
			this->cp.moveUp();
		else if ((char)ch=='l')
			this->cp.moveRight();
		else if ((char)ch=='h')
			this->cp.moveLeft();
		else if ((char)ch=='i')
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
		else if ((char)ch=='a')
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
		else if ((char)ch==':')
		{
			this->m=EX;
			this->message=":";
		}
	}
	else
	{
		if (ch==83+256)
		{
			// del key
			int currentLineLen=size(this->cp.linePos->line);
			if (this->cp.charPos>=currentLineLen)
				this->cp.charPos=currentLineLen-1;
			// delete the character under the cursor
			if (currentLineLen!=0)
				this->cp.linePos->line.erase(this->cp.charPos, 1);
			else
				this->cp.charPos=0;
			if (this->cp.charPos==currentLineLen-1 && currentLineLen!=1)
				this->cp.charPos--;
		}
	}
}

void vim_r::takeActionInsert(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='\n')
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
		else if ((char)ch=='\b')
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
		else if (ch==27)
		{
			// esc key
			this->m=NORMAL;
			int currentLineLen=size(this->cp.linePos->line);
			if (this->cp.charPos>=currentLineLen)
				this->cp.charPos=currentLineLen-1;
			else if (this->cp.charPos!=0)
				this->cp.charPos--;
		}
		else
		{
			// all other chars
			// just insert char(ch)
			int currentLineLen=size(this->cp.linePos->line);
			if (this->cp.charPos>currentLineLen)
				this->cp.charPos=currentLineLen;
			if (this->cp.charPos==currentLineLen) // cursor at the end of the line
				this->cp.linePos->line+=(char)ch;
			else
				this->cp.linePos->line.insert(this->cp.charPos, 1, (char)ch);
			this->cp.charPos++;
		}
	}
	else
	{
		if (ch==83+256)
		{
			// del key
			this->takeActionInsert('\b');
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
	}
}

void vim_r::takeActionEx(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='\n')
		{
			// deal with command
			takeActionEx(this->message);
		}
		else if ((char)ch=='\t')
		{
			// do nothing for now
		}
		else
			this->message+=(char)ch;
	}
	else
	{
		if (ch==83+256)
		{
			// del key
			this->takeActionEx('\b');
		}
	}
}

void vim_r::takeActionEX(string message)
{
	// delete extra spaces and leading colon in the message
	message.erase(0, 1);
	for (int i=1;i<size(message);++i)
	{
		if (message[i-1]==' ' && message[i]==' ')
		{
			message.erase(i, 1);
			i--;
		}
	}

	// take actions
}
