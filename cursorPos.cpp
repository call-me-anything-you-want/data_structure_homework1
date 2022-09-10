#include"header.h"
cursorPos::cursorPos()
{
	this->linePos=nullptr;
	this->charPos=0;
}

cursorPos::cursorPos(const cursorPos &c)
{
	this->linePos=c.linePos;
	this->charPos=c.charPos;
}

void cursorPos::moveCursor(mode m, direction d)
{
	if (m==NORMAL)
	{
		if (d==UP)
		{
			if (this->linePos==nullptr)
				return;
			this->linePos=this->linePos->prev==nullptr ? this->linePos : this->linePos->prev;
		}
		else if (d==DOWN)
		{
			if (this->linePos==nullptr)
				return;
			this->linePos=this->linePos->next==nullptr ? this->linePos : this->linePos->next;
		}
		else if (d==RIGHT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>=lineLen-1)
				this->charPos=lineLen-1;
			else
				this->charPos=this->charPos+1;
		}
		else if (d==LEFT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>=lineLen)
				this->charPos=lineLen-1;
			this->charPos=this->charPos==0 ? 0 : this->charPos-1;
		}
		else if (d==NONE)
		{
			int currentLineLen=this->linePos->line.size();
			if (currentLineLen==0)
				this->charPos=0;
			else if (this->charPos>=currentLineLen)
				this->charPos=currentLineLen-1;
		}
	}
	else if (m==INSERT)
	{
		if (d==RIGHT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>=lineLen)
				this->charPos=lineLen;
			else
				this->charPos=this->charPos+1;
		}
		else if (d==LEFT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>lineLen)
				this->charPos=lineLen-1;
			else
				this->charPos=this->charPos==0 ? 0 : this->charPos-1;
		}
		else if (d==NONE)
		{
			int currentLineLen=this->linePos->line.size();
			if (this->charPos>currentLineLen)
				this->charPos=currentLineLen;
		}
	}
	else if (m==VISUAL)
	{
		if (d==UP)
		{
			if (this->linePos==nullptr)
				return;
			this->linePos=this->linePos->prev==nullptr ? this->linePos : this->linePos->prev;
		}
		else if (d==DOWN)
		{
			if (this->linePos==nullptr)
				return;
			this->linePos=this->linePos->next==nullptr ? this->linePos : this->linePos->next;
		}
		else if (d==RIGHT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>=lineLen)
				this->charPos=lineLen;
			else
				this->charPos=this->charPos+1;
		}
		else if (d==LEFT)
		{
			int lineLen=this->linePos->line.size();
			if (lineLen==0)
				this->charPos=0;
			else if (this->charPos>lineLen)
				this->charPos=lineLen;
			this->charPos=this->charPos==0 ? 0 : this->charPos-1;
		}
	}
}
