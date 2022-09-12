#include "header.h"
environment::environment()
{
	this->ft=nullptr;
	this->changed=false;
}
environment::environment(const environment &e)
{
	this->ft=copyAll(e.ft);
	this->changed=e.changed;
}
environment::~environment()
{
	deleteAll(ft);
}
