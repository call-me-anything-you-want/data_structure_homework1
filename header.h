#ifndef HEADER
#define HEADER
enum mode {NORMAL, INSERT, EX};
class fileContent
{
};
class vim_r
{
	public:
		fileContent ft;
		mode m;
		vim_r(char * filename);
		void run();
		void takeAction(char ch);
};
#endif
