#include"header.h"
#include<string>
using namespace std;
fileContent::fileContent(string line) : line(line)
{
	this->prev=nullptr;
	this->next=nullptr;
}
