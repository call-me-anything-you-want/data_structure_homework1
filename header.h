#ifndef HEADER
#define HEADER
#include<string>
enum mode {NORMAL, INSERT, EX, VISUAL};
enum direction {UP, DOWN, LEFT, RIGHT, NONE};
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
void deleteAll(fileContent *f); // delete the link list starts from f
class cursorPos
{
	public:
		cursorPos();
		cursorPos(const cursorPos &c);
		// move the cursor under different modes, if the direction is NONE, it means to set the cursor's charPos to it's true value
		void moveCursor(mode m, direction d);
		// the cursor is currently at linePos->line[charPos]
		fileContent *linePos;
		int charPos;
};
class vim_r
{
	public:
		std::string filename; // the name of the file
		fileContent *ft; // records the file content currently being displayed in the window
		fileContent *clipBoard; // used for copy and paste
		cursorPos cp; // records the position of the cursor
		cursorPos visualCursor; // visual mode nees 2 cursors, one is the current cursor, the other is stored in visualCursor
		mode m; // records current mode
		bool changed; // true if the file is changed
		std::string message; // record the message needs to be displayed at the bottom, including input command in EX mode
		vim_r(char * filename);
		void run();
		void takeAction(int ch);
		void takeActionNormal(int ch);
		void takeActionInsert(int ch);
		void takeActionVisual(int ch);
		void takeActionEx(int ch); // this one deal with key input
		void takeActionEX(std::string message); // this one deal with line input
};
#endif
