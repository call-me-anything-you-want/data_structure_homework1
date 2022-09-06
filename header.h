#ifndef HEADER
#define HEADER
#include<string>
enum mode {NORMAL, INSERT, EX, VISUAL};
const int displayRowNum=10;
const int displayColNum=20;
class fileContent
{
	public:
		fileContent(std::string line="");
		std::string line;
		fileContent *prev;
		fileContent *next;
};
class cursorPos
{
	public:
		cursorPos();
		fileContent *linePos;
		int charPos;
};
class vim_r
{
	public:
		fileContent *ft;
		cursorPos cp;
		mode m;
		vim_r(char * filename);
		void run();
		void takeAction(char ch);
		void takeActionNormal(char ch);
		void takeActionInsert(char ch);
		void takeActionVisual(char ch);
		void takeActionEx(char ch);
};
#endif
