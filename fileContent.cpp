#include"header.h"
#include<string>
using namespace std;
fileContent::fileContent(string line) : line(line)
{
	this->prev=nullptr;
	this->next=nullptr;
}

bool fileContent::emptyLine()
{
	return size(this->line)==0;
}
