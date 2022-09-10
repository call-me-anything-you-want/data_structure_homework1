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
	return this->line.size()==0;
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

fileContent *copyAll(fileContent *f)
{
	if (f==nullptr)
		return nullptr;
	fileContent *newFile=new fileContent(f->line);
	fileContent *f1=newFile, *f2=f;
	while (f2->next!=nullptr)
	{
		f2=f2->next;
		fileContent *temp=new fileContent(f2->line);
		f1->next=temp;
		temp->prev=f1;
		f1=temp;
	}
	return newFile;
}
