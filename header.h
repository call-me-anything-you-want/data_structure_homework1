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
		std::string line; // records the content in the current line
		fileContent *prev; // pointer to previous line
		fileContent *next; // pointer to next line
		bool emptyLine();
};
class cursorPos
{
	public:
		cursorPos();
		// the following movements are movements in normal mode. movements in insert mode is a little bit different
		void moveUp();
		void moveDown();
		void moveLeft();
		void moveRight();
		// the cursor is currently at linePos->line[charPos]
		fileContent *linePos;
		int charPos;
};
class vim_r
{
	public:
		fileContent *ft; // records the file content currently being displayed in the window
		cursorPos cp; // records the position of the cursor
		mode m; // records current mode
		vim_r(char * filename);
		void run();
		void takeAction(int ch);
		void takeActionNormal(int ch);
		void takeActionInsert(int ch);
		void takeActionVisual(int ch);
		void takeActionEx(int ch);
};
#endif
