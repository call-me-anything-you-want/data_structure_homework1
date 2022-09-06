#include"header.h"
cursorPos::cursorPos()
{
	this->linePos=nullptr;
	this->charPos=0;
}

void cursorPos::moveUp()
{
	if (this->linePos==nullptr)
		return;
	this->linePos=this->linePos->prev==nullptr ? this->linePos : this->linePos->prev;
}

void cursorPos::moveDown()
{
	if (this->linePos==nullptr)
		return;
	this->linePos=this->linePos->next==nullptr ? this->linePos : this->linePos->next;
}

void cursorPos::moveRight()
{
	int lineLen=size(this->linePos->line);
	if (lineLen==0)
		this->charPos=0;
	else if (this->charPos>=lineLen-1)
		this->charPos=lineLen-1;
	else
		this->charPos=this->charPos+1;
}
void cursorPos::moveLeft()
{
	int lineLen=size(this->linePos->line);
	if (lineLen==0)
		this->charPos=0;
	else if (this->charPos>=lineLen)
		this->charPos=lineLen-1;
	this->charPos=this->charPos==0 ? 0 : this->charPos-1;
}
