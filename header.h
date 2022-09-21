#ifndef HEADER
#define HEADER
#include<string>
#include<Windows.h>
#include<vector>
enum mode {NORMAL, INSERT, COMMAND, VISUAL, REPLACE};
enum direction {UP, DOWN, LEFT, RIGHT, NONE};
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
fileContent *copyAll(fileContent *f); // return a link list that has the same content as f
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
class environment
{
	public:
		fileContent *ft;
		bool changed;
		environment();
		environment(const environment &e);
		~environment();
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
		bool currentCursorAhead; // true if the current cursor is before the visualCursor
		std::string message; // record the message needs to be displayed at the bottom, including input command in COMMAND mode
		std::string lastSearch; // record the last searching command
		std::vector<environment> historyEnvironment;
		int currrentEnvironmentIndex;
		HANDLE hOutBuffer[2]; // use buffer to display
		vim_r(char * filename);
		void run();
		void takeAction(int ch);
		void takeActionNormal(int ch);
		void takeActionInsert(int ch);
		void takeActionVisual(int ch);
		void takeActionCommand(int ch); // this one deal with key input
		void takeActionCommand(std::string message); // this one deal with line input
		void takeActionReplace(int ch);
		void display(int activeBuffer);
		void loadEnvironment(int index);
		void saveEnvironment();
};
#endif
