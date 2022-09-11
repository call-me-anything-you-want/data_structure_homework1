#include "header.h"
#include<fstream>
#include<iostream>
#include<string>
#include<conio.h>
#include<vector>
#include<Windows.h>
using namespace std;
vim_r::vim_r(char *filename) : cp(), changed(false), currentCursorAhead(false), clipBoard(nullptr)
{
	if (filename==nullptr)
		this->filename="";
	else
		this->filename=string(filename);
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
			this->ft=new fileContent();
			this->cp.linePos=this->ft;
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
			fin.close();
		}
	}
	else
	{
		this->ft=new fileContent();
		this->cp.linePos=this->ft;
	}
	this->visualCursor=cursorPos(this->cp);
}

void vim_r::run()
{
	// 双缓冲变量
	HANDLE hOutBuffer[2];
	//缓冲区初始化
	hOutBuffer[0] = CreateConsoleScreenBuffer(
			GENERIC_WRITE,//定义进程可以往缓冲区写数据
			FILE_SHARE_WRITE,//定义缓冲区可以共享写入权限
			NULL,
			CONSOLE_TEXTMODE_BUFFER,
			NULL
			);
	hOutBuffer[1] = CreateConsoleScreenBuffer(
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CONSOLE_TEXTMODE_BUFFER,
			NULL
			);
	//选择隐藏缓冲区光标可见性
	CONSOLE_CURSOR_INFO cci;
	cci.bVisible = 0;
	cci.dwSize = 1;
	SetConsoleCursorInfo(hOutBuffer[0], &cci);
	SetConsoleCursorInfo(hOutBuffer[1], &cci);
	int count=0;
	for (;;)
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
		this->display(hOutBuffer, count);
		count=(count+1)%14;
	}
}

void vim_r::display(HANDLE *hOutBuffer, int count)
{
	int activeBuffer=count%2;
	COORD coord = { 0,0 };
	DWORD bytes = 0;
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], this->filename.data(), this->filename.size(), coord, &bytes);
	coord.Y+=1;
	if (this->changed)
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], "changed", 7, coord, &bytes);
	else
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], "unchanged", 9, coord, &bytes);
	coord.Y+=2;
	fileContent *temp=this->ft;
	while (temp!=nullptr)
	{
		string currentLine=temp->line;
		if (temp==this->cp.linePos && count<7)
		{
			if (this->m==NORMAL)
			{
				if (currentLine.size()==0)
					currentLine= "█";
				else if (this->cp.charPos>=currentLine.size())
					currentLine.replace(currentLine.size()-1, 1, "█");
				else
					currentLine.replace(this->cp.charPos, 1,  "█");
			}
			else if (this->m==INSERT || this->m==REPLACE)
			{
				if (currentLine.size()==0)
					currentLine= "▁";
				else if (this->cp.charPos>=currentLine.size())
					currentLine+= "▁";
				else
					currentLine.replace(this->cp.charPos, 1,  "▁");
			}
			else if (this->m==VISUAL)
			{
				if (currentLine.size()==0)
					currentLine= "█";
				else if (this->cp.charPos>=currentLine.size())
					currentLine+= "█";
				else
					currentLine.replace(this->cp.charPos, 1,  "█");
			}
		}
		if (temp==this->visualCursor.linePos)
		{
			if (this->m==VISUAL)
			{
				if (currentLine.size()==0)
					currentLine= "█";
				else if (this->cp.charPos>=currentLine.size())
					currentLine+= "█";
				else
					currentLine.replace(this->cp.charPos, 1,  "█");
			}
		}
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], currentLine.data(), currentLine.size(), coord, &bytes);
		temp=temp->next;
		coord.Y++;
	}
	coord.Y++;
	string currentMode;
	if (this->m==NORMAL)
		currentMode="--NORMAL--";
	else if (this->m==INSERT)
		currentMode="--INSERT--";
	else if (this->m==VISUAL)
		currentMode="--VISUAL--";
	else if (this->m==EX)
		currentMode="--COMMAND--";
	else if (this->m==REPLACE)
		currentMode="--REPLACE--";
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], currentMode.data(), currentMode.size(), coord, &bytes);
	coord.Y++;
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], this->message.data(), this->message.size(), coord, &bytes);
	SetConsoleActiveScreenBuffer(hOutBuffer[activeBuffer]);//设置新的缓冲区为活动显示缓冲
	Sleep(50);

	int nextHandleIndex=(activeBuffer+1)%2;
	CloseHandle(hOutBuffer[nextHandleIndex]);
	hOutBuffer[nextHandleIndex] = CreateConsoleScreenBuffer(
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CONSOLE_TEXTMODE_BUFFER,
			NULL
			);
	//选择隐藏缓冲区光标可见性
	CONSOLE_CURSOR_INFO cci;
	cci.bVisible = 0;
	cci.dwSize = 1;
	SetConsoleCursorInfo(hOutBuffer[nextHandleIndex], &cci);
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
	else if (this->m==REPLACE)
		takeActionReplace(ch);
}

void vim_r::takeActionNormal(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='j')
			this->cp.moveCursor(NORMAL, DOWN);
		else if ((char)ch=='k')
			this->cp.moveCursor(NORMAL, UP);
		else if ((char)ch=='l')
			this->cp.moveCursor(NORMAL, RIGHT);
		else if ((char)ch=='h')
			this->cp.moveCursor(NORMAL, LEFT);
		else if ((char)ch=='x')
		{
			// delete the char under the cursor
			this->cp.moveCursor(NORMAL, NONE);
			int currentLineLen=this->cp.linePos->line.size();
			if (currentLineLen!=0)
			{
				// copy the deleting char into the clip board
				deleteAll(this->clipBoard);
				this->clipBoard=new fileContent();
				this->clipBoard->line=this->cp.linePos->line[this->cp.charPos];
				// delete the char
				this->cp.linePos->line.erase(this->cp.charPos, 1);
				this->changed=true;
			}
			else
				this->cp.charPos=0;
			// if deleted the last character, cursor needs to be moved back
			this->cp.moveCursor(NORMAL, NONE);
		}
		else if ((char)ch=='p')
		{
			// paste the clipboard
			this->cp.moveCursor(NORMAL, NONE);
			if (this->clipBoard!=nullptr)
			{
				fileContent *pasteContent=copyAll(this->clipBoard);
				fileContent *pasteTail=pasteContent;
				while (pasteTail->next!=nullptr)
					pasteTail=pasteTail->next;
				if (this->ft==nullptr)
				{
					this->ft=new fileContent();
					this->cp.linePos=ft;
				}
				string s1, s2;
				int lineLen=this->cp.linePos->line.size();
				s1=lineLen==0 ? "" : this->cp.linePos->line.substr(0, this->cp.charPos+1);
				s2=this->cp.charPos==lineLen-1 ? "" : this->cp.linePos->line.substr(this->cp.charPos+1);
				pasteContent->line=s1+pasteContent->line;
				pasteTail->line+=s2;
				pasteContent->prev=this->cp.linePos->prev;
				pasteTail->next=this->cp.linePos->next;
				if (pasteContent->prev!=nullptr)
					pasteContent->prev->next=pasteContent;
				if (pasteTail->next!=nullptr)
					pasteTail->next->prev=pasteTail;
				if (this->ft==this->cp.linePos)
					this->ft=pasteContent;
				delete this->cp.linePos;
				this->cp.linePos=pasteContent;
				this->cp.moveCursor(NORMAL, RIGHT);
			}
		}
		else if ((char)ch=='i')
		{
			this->m=INSERT;
			if (this->ft==nullptr)
			{
				this->ft=new fileContent();
				this->cp.linePos=this->ft;
				this->cp.charPos=0;
			}
			else
				this->cp.moveCursor(NORMAL, NONE);
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
				this->cp.moveCursor(INSERT, RIGHT);
		}
		else if ((char)ch=='R')
		{
			this->m=REPLACE;
			this->cp.moveCursor(NORMAL, NONE);
		}
		else if ((char)ch=='v')
		{
			// enter visual mode
			this->cp.moveCursor(NORMAL, NONE);
			this->m=VISUAL;
			this->visualCursor=this->cp;
			this->currentCursorAhead=false;
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
			this->takeActionNormal((int)'x');
		}
	}
}

void vim_r::takeActionInsert(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='\r')
		{
			this->changed=true;
			// create a new line
			this->cp.moveCursor(INSERT, NONE);
			int currentLineLen=this->cp.linePos->line.size();
			string newline;
			if (this->cp.charPos==currentLineLen)
				newline="";
			else
			{
				newline=this->cp.linePos->line.substr(this->cp.charPos);
				this->cp.linePos->line=this->cp.linePos->line.substr(0, this->cp.charPos);
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
			// delete a character before the cursor
			this->cp.moveCursor(INSERT, NONE);
			int currentLineLen=this->cp.linePos->line.size();
			if (this->cp.charPos!=0) // if the cursor is not at the front of this line, just delete a char
			{
				this->changed=true;
				this->cp.moveCursor(INSERT, LEFT);
				this->cp.linePos->line.erase(this->cp.charPos, 1);
			}
			else
			{
				if (this->cp.linePos->prev!=nullptr) // if the cursor is not at the front of the whole file
				{
					this->changed=true;
					this->cp.charPos=this->cp.linePos->prev->line.size();
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
			this->cp.moveCursor(INSERT, LEFT);
		}
		else
		{
			// all other chars
			// just insert char(ch)
			this->changed=true;
			this->cp.moveCursor(INSERT, NONE);
			int currentLineLen=this->cp.linePos->line.size();
			if (this->cp.charPos==currentLineLen) // cursor at the end of the line
				this->cp.linePos->line+=(char)ch;
			else
				this->cp.linePos->line.insert(this->cp.charPos, 1, (char)ch);
			this->cp.moveCursor(INSERT, RIGHT);
		}
	}
	else
	{
		if (ch==83+256)
		{
			// del key
			// delete a character under the cursor
			this->cp.moveCursor(INSERT, NONE);
			int currentLineLen=this->cp.linePos->line.size();
			if (this->cp.charPos<currentLineLen)
			{
				this->changed=true;
				this->cp.linePos->line.erase(this->cp.charPos, 1);
			}
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
		if ((char)ch=='\r')
		{
			// deal with command
			this->takeActionEX(this->message);
			this->m=NORMAL;
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

void vim_r::takeActionEX(string EXmessage)
{
	// delete extra spaces and leading colon in the EXmessage
	if (EXmessage[0]==':')
		EXmessage.erase(0, 1);
	for (int i=EXmessage.size()-1;i>=0;i--)
	{
		if (EXmessage[i]==' ')
			EXmessage.erase(i, 1);
		else
			break;
	}
	for (int i=0;i<EXmessage.size();i++)
	{
		if (EXmessage[i]==' ')
		{
			EXmessage.erase(i, 1);
			i--;
		}
		else
			break;
	}
	for (int i=1;i<EXmessage.size();++i)
	{
		if (EXmessage[i-1]==' ' && EXmessage[i]==' ')
		{
			EXmessage.erase(i, 1);
			i--;
		}
	}

	if (EXmessage=="")
		return;

	// find the command
	string command="";
	vector<string> parameters(0);
	int beg=0, end=0;
	while(end<EXmessage.size() && EXmessage[end]!=' ' && EXmessage[end]!='/')
		++end;
	command=EXmessage.substr(0, end);
	if (end<EXmessage.size()-1)
		EXmessage=EXmessage.substr(end+1);
	else
		EXmessage="";

	// take actions
	if (command=="w" || command=="write")
	{
		// write
		if (EXmessage.size()==0)
		{
			if (this->filename=="")
				this->message="No file name.";
			else if (this->changed)
			{
				// just write the current file
				// delete the file content before writing
				fstream fout(this->filename, ios::out|ios::trunc);
				fileContent *temp=this->ft;
				while (temp!=nullptr)
				{
					fout << temp->line;
					temp=temp->next;
					if (temp!=nullptr)
						fout << endl;
				}
				fout.close();
				this->message=this->filename+"has been written.";
				this->changed=false;
			}
			else
				this->message=this->filename+"has been written.";
		}
		else
		{
			// write the current file to file EXmessage
			fstream fout(EXmessage, ios::out|ios::trunc);
			fileContent *temp=this->ft;
			while (temp!=nullptr)
			{
				fout << temp->line;
				temp=temp->next;
				if (temp!=nullptr)
					fout << endl;
			}
			fout.close();
			this->filename=EXmessage;
			this->message=this->filename+" has been written.";
			this->changed=false;
		}
	}
	else if(command=="q" || command=="quit")
	{
		// quit
		if (!this->changed)
			exit(0);
		else
			this->message="No write since last change.";
	}
	else if (command=="q!")
	{
		// quit with out save
		exit(0);
	}
	else if (command=="wq")
	{
		this->takeActionEX(":w");
		this->takeActionEX(":q");
	}
}

void vim_r::takeActionVisual(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='h')
		{
			this->cp.moveCursor(VISUAL, LEFT);
			if (this->currentCursorAhead==false && this->cp.linePos==this->visualCursor.linePos && this->cp.charPos<this->visualCursor.charPos)
				this->currentCursorAhead=true;
		}
		else if ((char)ch=='j')
		{
			this->cp.moveCursor(VISUAL, DOWN);
			if (this->currentCursorAhead==true && this->cp.linePos==this->visualCursor.linePos->next)
				currentCursorAhead=false;
			else if (this->currentCursorAhead==true && this->cp.linePos==this->visualCursor.linePos && this->cp.charPos>=this->visualCursor.charPos)
				currentCursorAhead=false;
		}
		else if ((char)ch=='k')
		{
			this->cp.moveCursor(VISUAL, UP);
			if (this->currentCursorAhead==false && this->cp.linePos==this->visualCursor.linePos->prev)
				currentCursorAhead=true;
			else if (this->currentCursorAhead==false && this->cp.linePos==this->visualCursor.linePos && this->cp.charPos<this->visualCursor.charPos)
				currentCursorAhead=true;
		}
		else if ((char)ch=='l')
		{
			this->cp.moveCursor(VISUAL, RIGHT);
			if (this->currentCursorAhead==true && this->cp.linePos==this->visualCursor.linePos && this->cp.charPos>=this->visualCursor.charPos)
				this->currentCursorAhead=false;
		}
		else if ((char)ch=='d')
		{
			// delete
			this->takeActionVisual((int)'y');
			cursorPos beg, end;
			if (this->currentCursorAhead)
			{
				beg.charPos=this->cp.charPos;
				beg.linePos=this->cp.linePos;
				end.charPos=this->visualCursor.charPos;
				end.linePos=this->visualCursor.linePos;
			}
			else
			{
				beg.charPos=this->visualCursor.charPos;
				beg.linePos=this->visualCursor.linePos;
				end.charPos=this->cp.charPos;
				end.linePos=this->cp.linePos;
			}
			if (beg.linePos==end.linePos)
			{
				int currentLineLen=beg.linePos->line.size();
				if (beg.charPos!=currentLineLen)
				{
					beg.linePos->line.erase(beg.charPos, end.charPos-beg.charPos+1);
					this->changed=true;
				}
				if (end.charPos==currentLineLen)
				{
					if (beg.linePos->next!=nullptr)
					{
						beg.linePos->line+=beg.linePos->next->line;
						fileContent *temp=beg.linePos->next;
						beg.linePos->next=beg.linePos->next->next;
						delete temp;
						if (beg.linePos->next!=nullptr)
							beg.linePos->next->prev=beg.linePos;
						this->changed=true;
					}
				}
			}
			else
			{
				int begLineLen=beg.linePos->line.size();
				if (beg.charPos!=begLineLen)
					beg.linePos->line.erase(beg.charPos);
				fileContent *temp=beg.linePos->next;
				while (temp!=end.linePos)
				{
					temp->prev->next=temp->next;
					temp->next->prev=temp->prev;
					fileContent *deleting=temp;
					temp=temp->next;
					delete deleting;
				}
				int endLineLen=end.linePos->line.size();
				end.linePos->line=end.linePos->line.erase(0, end.charPos+1);
				if (end.charPos==endLineLen)
				{
					if (end.linePos->next!=nullptr)
					{
						end.linePos->line+=end.linePos->next->line;
						fileContent *deleting=end.linePos->next;
						end.linePos->next=deleting->next;
						if (end.linePos->next!=nullptr)
							end.linePos->next->prev=end.linePos;
						delete deleting;
					}
				}
				if (beg.charPos==begLineLen)
				{
					beg.linePos->line+=end.linePos->line;
					beg.linePos->next=end.linePos->next;
					if (beg.linePos->next!=nullptr)
						beg.linePos->next->prev=beg.linePos;
					delete end.linePos;
				}
			}
			this->cp.linePos=beg.linePos;
			this->cp.charPos=beg.charPos;
			this->cp.moveCursor(NORMAL, NONE);
			this->m=NORMAL;
		}
		else if ((char)ch=='x')
			this->takeActionVisual((int)'d');
		else if ((char)ch=='y')
		{
			// yank
			cursorPos beg, end;
			if (this->currentCursorAhead)
			{
				beg.charPos=this->cp.charPos;
				beg.linePos=this->cp.linePos;
				beg.moveCursor(VISUAL, NONE);
				end.charPos=this->visualCursor.charPos;
				end.linePos=this->visualCursor.linePos;
				end.moveCursor(VISUAL, NONE);
			}
			else
			{
				beg.charPos=this->visualCursor.charPos;
				beg.linePos=this->visualCursor.linePos;
				beg.moveCursor(VISUAL, NONE);
				end.charPos=this->cp.charPos;
				end.linePos=this->cp.linePos;
				end.moveCursor(VISUAL, NONE);
			}
			deleteAll(this->clipBoard);
			fileContent *tail;
			if (beg.linePos==end.linePos)
			{
				int currentLineLen=beg.linePos->line.size();
				if (beg.charPos==currentLineLen)
					this->clipBoard=new fileContent();
				else
					this->clipBoard=new fileContent(beg.linePos->line.substr(beg.charPos, end.charPos-beg.charPos+1));
				tail=this->clipBoard;
			}
			else
			{
				int currentLineLen=beg.linePos->line.size();
				if (beg.charPos==currentLineLen)
					this->clipBoard=new fileContent();
				else
					this->clipBoard=new fileContent(beg.linePos->line.substr(beg.charPos));
				beg.linePos=beg.linePos->next;
				tail=this->clipBoard;
				while (beg.linePos!=end.linePos)
				{
					tail->next=new fileContent(beg.linePos->line);
					tail->next->prev=tail;
					beg.linePos=beg.linePos->next;
					tail=tail->next;
				}
				tail->next=new fileContent(end.linePos->line.substr(0, end.charPos+1));
				tail->next->prev=tail;
				tail=tail->next;
			}
			int endLineLen=end.linePos->line.size();
			if (end.charPos==endLineLen)
			{
				tail->next=new fileContent();
				tail->next->prev=tail;
			}
			// put the cursor to the front
			this->cp.linePos=beg.linePos;
			this->cp.charPos=beg.charPos;
			this->m=NORMAL;
		}
		else if (ch==27)
		{
			// esc
			this->m=NORMAL;
		}
	}
	else
	{
		if (ch==83+256)
		{
			// del key
			this->takeActionVisual((int)'d');
		}
	}
}


void vim_r::takeActionReplace(int ch)
{
	if (ch<=127)
	{
		if (ch==8)
		{
			// BS
			this->cp.moveCursor(REPLACE, LEFT);
		}
		else if (ch==27)
		{
			// esc
			this->m=NORMAL;
			this->cp.moveCursor(NORMAL, NONE);
		}
		else
		{
			this->changed=true;
			int currentLineLen=this->cp.linePos->line.size();
			if (this->cp.charPos>=currentLineLen)
				this->cp.linePos->line+=(char)ch;
			else
				this->cp.linePos->line[this->cp.charPos]=(char)ch;
			this->cp.moveCursor(REPLACE, RIGHT);
		}
	}
	else
	{
		if (ch==83+256)
		{
			// del
			this->takeActionInsert(83+256);
		}
	}
}
