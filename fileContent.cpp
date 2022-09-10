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

void deleteAll(fileContent *f)
{
	if (f==nullptr)
		return;
	fileContent *temp=f->next;
	delete f;
	while (temp!=nullptr)
	{
		f=temp;
		temp=temp->next;
		delete f;
	}
}
