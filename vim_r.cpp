#include "header.h"
#include<fstream>
#include<iostream>
#include<string>
#include<conio.h>
#include<vector>
#include<Windows.h>
using namespace std;
vim_r::vim_r(char *filename) : cp(), changed(false), currentCursorAhead(false), clipBoard(nullptr), historyEnvironment(vector<environment>()), currrentEnvironmentIndex(-1), lastSearch("")
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
			this->filename=string(filename);
			this->message="\"" + this->filename + "\" [new]";
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
	this->saveEnvironment();
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
	SetConsoleTitleA("vim_r");
	for (;;)
	{
		if (_kbhit())
		{
			char ch=_getch();
			if (ch>0)
			{
				this->takeAction((int)ch);
			}
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

	// get rows and columns of the current buffer
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	int headRows=3; // the number of rows unrelated to text at the head of the buffer
	int tailRows=3; // the number of rows unrelated to text at the tail of the buffer
	int textRows=rows-headRows-tailRows;
	int textColumns=columns-2;

	COORD coord = { 0,0 };
	DWORD bytes = 0;

	if (rows<headRows+tailRows+1)
	{
		coord.Y=rows-1;
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], "window too small", 16, coord, &bytes);
		return;
	}

	// filename and changed
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], this->filename.data(), min((int)this->filename.size(), columns), coord, &bytes);
	coord.Y+=1;
	if (this->changed)
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], "changed", min(7, columns), coord, &bytes);
	else
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], "unchanged", min(9, columns), coord, &bytes);
	coord.Y+=1;
	string splitLine=string(columns, '=');
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], splitLine.data(), columns, coord, &bytes);

	// file content
	fileContent *temp=this->cp.linePos;
	coord.Y=(textRows+1)/2+headRows-1;
	cursorPos tempCursor;
	tempCursor.linePos=this->cp.linePos;
	tempCursor.charPos=this->cp.charPos;
	tempCursor.moveCursor(this->m, NONE);
	int currentBeg=tempCursor.charPos/textColumns*textColumns;
	while (coord.Y>headRows-1)
	{
		// print
		string currentLine=temp->line.substr(currentBeg, textColumns);

		// change the text to cursor if needed
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
			if (this->cp.linePos!=temp || (this->cp.charPos!=this->visualCursor.charPos && (this->cp.charPos<=temp->line.size() || this->visualCursor.charPos<=temp->line.size())))
			{
				// only display this when the two cursor is not at the same position
				if (this->m==VISUAL)
				{
					if (currentLine.size()==0)
						currentLine= "█";
					else if (this->cp.linePos!=temp || this->currentCursorAhead==false || count>=7)
					{
						// the two cursor is not in the same line or cp is behind the visualCursor or cp is not displayed
						if (this->visualCursor.charPos>=currentLine.size())
							currentLine+="█";
						else
							currentLine.replace(this->visualCursor.charPos, 1,  "█");
					}
					else
					{
						if (this->visualCursor.charPos>=currentLine.size()-2)
							currentLine+="█";
						else
							currentLine.replace(this->visualCursor.charPos+2, 1, "█");
					}
				}
			}
		}

		// output
		if (currentBeg==0)
			currentLine="* "+currentLine;
		else
			currentLine="  "+currentLine;
		WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], currentLine.data(), currentLine.size(), coord, &bytes);

		// prepare for next iteration
		if (currentBeg!=0)
			currentBeg-=textColumns;
		else
		{
			temp=temp->prev;
			if (temp==nullptr)
				break;
			currentBeg=temp->line.size()/textColumns*textColumns;
		}

		coord.Y--;
	}

	coord.Y=(textRows+1)/2+headRows;
	temp=this->cp.linePos;
	currentBeg=tempCursor.charPos/textColumns*textColumns+textColumns;
	if (currentBeg>=this->cp.linePos->line.size())
	{
		currentBeg=0;
		temp=temp->next;
	}
	if (temp!=nullptr)
	{
		while (coord.Y<rows-tailRows)
		{
			// print
			string currentLine=temp->line.substr(currentBeg, textColumns);

			if (currentBeg==0)
				currentLine="* "+currentLine;
			else
				currentLine="  "+currentLine;
			WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], currentLine.data(), currentLine.size(), coord, &bytes);

			// prepare for next iteration
			currentBeg+=textColumns;
			if (currentBeg>=temp->line.size())
			{
				temp=temp->next;
				if (temp==nullptr)
					break;
				currentBeg=0;
			}
			coord.Y++;
		}
	}

	// mode and message
	coord.Y=rows-tailRows;
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], splitLine.data(), columns, coord, &bytes);
	coord.Y++;
	string currentMode;
	if (this->m==NORMAL)
		currentMode="--NORMAL--";
	else if (this->m==INSERT)
		currentMode="--INSERT--";
	else if (this->m==VISUAL)
		currentMode="--VISUAL--";
	else if (this->m==COMMAND)
		currentMode="--COMMAND--";
	else if (this->m==REPLACE)
		currentMode="--REPLACE--";
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], currentMode.data(), currentMode.size(), coord, &bytes);
	coord.Y++;
	WriteConsoleOutputCharacter(hOutBuffer[activeBuffer], this->message.data(), this->message.size(), coord, &bytes);
	SetConsoleActiveScreenBuffer(hOutBuffer[activeBuffer]);//设置新的缓冲区为活动显示缓冲
	Sleep(50);

	// recreate the next handle to clear the screen
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
	else if (this->m==COMMAND)
		takeActionCommand(ch);
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
		else if ((char)ch=='0')
		{
			// go to the begin of the line
			if (this->cp.linePos==nullptr)
			{
				this->cp.linePos=new fileContent();
				this->ft=this->cp.linePos;
				this->cp.charPos=0;
			}
			else
				this->cp.charPos=0;
		}
		else if ((char)ch=='$')
		{
			// go to the end of the line
			if (this->cp.linePos==nullptr)
			{
				this->cp.linePos=new fileContent();
				this->ft=this->cp.linePos;
				this->cp.charPos=0;
			}
			else
				this->cp.charPos=this->cp.linePos->line.size()-1;
		}
		else if ((char)ch=='G')
		{
			// go to the end of the file
			if (this->cp.linePos==nullptr)
			{
				this->cp.linePos=new fileContent();
				this->ft=this->cp.linePos;
				this->cp.charPos=0;
			}
			else
			{
				while (this->cp.linePos->next!=nullptr)
					this->cp.linePos=this->cp.linePos->next;
				this->cp.charPos=0;
			}
		}
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
				this->saveEnvironment();
			}
			else
				this->cp.charPos=0;
			// if deleted the last character, cursor needs to be moved back
			this->cp.moveCursor(NORMAL, NONE);
		}
		else if ((char)ch=='~')
		{
			// change between upper case and lower case
			this->cp.moveCursor(NORMAL, NONE);
			if (this->cp.linePos->line[this->cp.charPos]>='a' && this->cp.linePos->line[this->cp.charPos]<='z')
			{
				this->cp.linePos->line[this->cp.charPos]-=32;
				this->changed=true;
			}
			else if (this->cp.linePos->line[this->cp.charPos]>='A' && this->cp.linePos->line[this->cp.charPos]<='Z')
			{
				this->cp.linePos->line[this->cp.charPos]+=32;
				this->changed=true;
			}
			this->cp.moveCursor(NORMAL, RIGHT);
			this->saveEnvironment();
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
				this->changed=true;
				this->saveEnvironment();
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
		else if ((char)ch=='o')
		{
			this->m=INSERT;
			if (this->ft==nullptr)
			{
				this->ft=new fileContent();
				this->cp.linePos=this->ft;
				this->cp.charPos=0;
			}
			fileContent *newline=new fileContent();
			newline->next=this->cp.linePos->next;
			if (newline->next!=nullptr)
				newline->next->prev=newline;
			this->cp.linePos->next=newline;
			newline->prev=this->cp.linePos;
			this->cp.linePos=newline;
			this->cp.charPos=0;
			this->changed=true;
			this->saveEnvironment();
		}
		else if ((char)ch=='O')
		{
			this->m=INSERT;
			if (this->ft==nullptr)
			{
				this->ft=new fileContent();
				this->cp.linePos=this->ft;
				this->cp.charPos=0;
			}
			fileContent *newline=new fileContent();
			newline->prev=this->cp.linePos->prev;
			if (newline->prev!=nullptr)
				newline->prev->next=newline;
			else
				this->ft=newline;
			this->cp.linePos->prev=newline;
			newline->next=this->cp.linePos;
			this->cp.linePos=newline;
			this->cp.charPos=0;
			this->changed=true;
			this->saveEnvironment();
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
			this->m=COMMAND;
			this->message=":";
		}
		else if ((char)ch=='/')
		{
			this->m=COMMAND;
			this->message="/";
		}
		else if ((char)ch=='?')
		{
			this->m=COMMAND;
			this->message="?";
		}
		else if ((char)ch=='n')
		{
			if (this->lastSearch.size()==0)
				return;
			this->message=this->lastSearch;
			this->takeActionCommand(this->lastSearch);
		}
		else if ((char)ch=='N')
		{
			if (this->lastSearch.size()==0)
				return;
			string temp=this->lastSearch;
			if (temp[0]=='/')
				temp[0]='?';
			else
				temp[0]='/';
			this->message=temp;
			this->takeActionCommand(temp);
		}
		else if ((char)ch=='u')
		{
			// undo
			if (this->currrentEnvironmentIndex==0)
				this->message="Has been in the oldest change.";
			else
				this->loadEnvironment(this->currrentEnvironmentIndex-1);
		}
		else if (ch==18)
		{
			// ctrl-r, redo
			if (this->currrentEnvironmentIndex==this->historyEnvironment.size()-1)
				this->message="Has been in the latest change.";
			else
				this->loadEnvironment(this->currrentEnvironmentIndex+1);
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
			this->saveEnvironment();
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

void vim_r::takeActionCommand(int ch)
{
	if (ch<=127)
	{
		if ((char)ch=='\r')
		{
			// deal with command
			this->takeActionCommand(this->message);
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
			this->takeActionCommand('\b');
		}
	}
}

void vim_r::takeActionCommand(string EXmessage)
{
	if (EXmessage[0]==':')
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
				this->saveEnvironment();
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
			this->takeActionCommand(":w");
			this->takeActionCommand(":q");
		}
	}
	else if (EXmessage[0]=='/')
	{
		this->lastSearch=EXmessage;
		// search after the current cursor
		string searchingString=EXmessage.substr(1);
		cursorPos currentCursor=this->cp;
		currentCursor.moveCursor(NORMAL, NONE);
		int pos=0;
		while (currentCursor.linePos!=nullptr)
		{
			if (pos>=currentCursor.linePos->line.size())
			{
				currentCursor.linePos=currentCursor.linePos->next;
				pos=0;
				continue;
			}
			pos=currentCursor.linePos->line.find(searchingString, pos);
			if (pos==string::npos)
			{
				currentCursor.linePos=currentCursor.linePos->next;
				pos=0;
			}
			else if (this->cp.linePos==currentCursor.linePos && pos<=currentCursor.charPos)
				pos+=searchingString.size();
			else
				break;
		}
		if (currentCursor.linePos!=nullptr)
		{
			// find one before reaching the end, jump to it
			this->cp=currentCursor;
			this->cp.charPos=pos;
			return;
		}
		this->message="The begin of the file has been reached, and then continue to search from the end";
		currentCursor.linePos=this->ft;
		pos=0;
		while (currentCursor.linePos!=this->cp.linePos)
		{
			if (pos>=currentCursor.linePos->line.size())
			{
				currentCursor.linePos=currentCursor.linePos->next;
				pos=0;
				continue;
			}
			pos=currentCursor.linePos->line.find(searchingString, pos);
			if (pos==string::npos)
			{
				currentCursor.linePos=currentCursor.linePos->next;
				pos=0;
			}
			else
				break;
		}
		if (currentCursor.linePos!=this->cp.linePos)
		{
			// find one before reaching the end, jump to it
			this->cp=currentCursor;
			this->cp.charPos=pos;
			return;
		}
		this->message="Pattern not found: "+searchingString;
	}
	else if (EXmessage[0]=='?')
	{
		this->lastSearch=EXmessage;
		// search before the current cursor
		string searchingString=EXmessage.substr(1);
		cursorPos currentCursor=this->cp;
		currentCursor.moveCursor(NORMAL, NONE);
		int pos=0;
		while (currentCursor.linePos!=nullptr)
		{
			vector<int> posFound;
			while (1)
			{
				pos=currentCursor.linePos->line.find(searchingString, pos);
				if (pos!=string::npos)
				{
					posFound.push_back(pos);
					pos+=searchingString.size();
				}
				else
					break;
			}
			if (posFound.size()==0)
			{
				currentCursor.linePos=currentCursor.linePos->prev;
				pos=0;
			}
			else if (currentCursor.linePos!=this->cp.linePos)
			{
				pos=posFound[posFound.size()-1];
				break;
			}
			else
			{
				pos=(int)string::npos;
				for (int i=posFound.size()-1;i>=0;i--)
				{
					if (posFound[i]<currentCursor.charPos)
					{
						pos=posFound[i];
						break;
					}
				}
				if (pos==string::npos)
				{
					currentCursor.linePos=currentCursor.linePos->prev;
					pos=0;
				}
				else
					break;
			}
		}
		if (currentCursor.linePos!=nullptr)
		{
			// find one before reaching the end, jump to it
			this->cp=currentCursor;
			this->cp.charPos=pos;
			return;
		}
		this->message="The end of the file has been reached, and then continue to search from the beginning";
		currentCursor.linePos=this->ft;
		while (currentCursor.linePos->next!=nullptr)
			currentCursor.linePos=currentCursor.linePos->next;
		pos=0;
		while (currentCursor.linePos!=this->cp.linePos)
		{
			vector<int> posFound;
			while (1)
			{
				pos=currentCursor.linePos->line.find(searchingString, pos);
				if (pos!=string::npos)
				{
					posFound.push_back(pos);
					pos+=searchingString.size();
				}
				else
					break;
			}
			if (posFound.size()==0)
			{
				currentCursor.linePos=currentCursor.linePos->prev;
				pos=0;
			}
			else
			{
				pos=posFound[posFound.size()-1];
				break;
			}
		}
		if (currentCursor.linePos!=this->cp.linePos)
		{
			// find one before reaching the end, jump to it
			this->cp=currentCursor;
			this->cp.charPos=pos;
			return;
		}
		this->message="Pattern not found: "+searchingString;
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
			this->takeActionVisual((int)'y');
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
				beg.linePos->line+=end.linePos->line;
				beg.linePos->next=end.linePos->next;
				if (beg.linePos->next!=nullptr)
					beg.linePos->next->prev=beg.linePos;
				delete end.linePos;
				this->changed=true;
			}
			this->cp.linePos=beg.linePos;
			this->cp.charPos=beg.charPos;
			this->cp.moveCursor(NORMAL, NONE);
			this->m=NORMAL;
			this->saveEnvironment();
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
				fileContent *temp=beg.linePos->next;
				tail=this->clipBoard;
				while (temp!=end.linePos)
				{
					tail->next=new fileContent(temp->line);
					tail->next->prev=tail;
					temp=temp->next;
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
			this->saveEnvironment();
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

void vim_r::saveEnvironment()
{
	while (this->currrentEnvironmentIndex!=this->historyEnvironment.size()-1)
		this->historyEnvironment.pop_back();
	environment e;
	this->historyEnvironment.push_back(e);
	this->currrentEnvironmentIndex++;
	this->historyEnvironment[this->currrentEnvironmentIndex].changed=this->changed;
	this->historyEnvironment[this->currrentEnvironmentIndex].ft=copyAll(this->ft);
}

void vim_r::loadEnvironment(int index)
{
	this->currrentEnvironmentIndex=index;
	this->changed=this->historyEnvironment[index].changed;
	deleteAll(this->ft);
	this->ft=copyAll(this->historyEnvironment[index].ft);
	this->cp.linePos=this->ft;
	this->cp.charPos=0;
	this->m=NORMAL;
}
