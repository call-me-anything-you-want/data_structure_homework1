# 数据结构课程设计报告
## 课程设计目的
设计一个简单的文本编辑器SimpleWord，可以打开存储在硬盘中的已有文件，并进行相关的编辑操作；也可以创建一个新的文件，保存到硬盘。

## 课程设计要求
1. 文件处理功能（FILE）
	* 新建文件（New）：若编辑区有未保存的编辑内容，询问是否保存后再清空编辑区。
	* 打开文件（Open）：要求用户输入文件名，该文件存在则打开载入编辑区，否则提示为“新文件”。
	* 保存文件（Save）：提示用户当前文件名，用户可以重置文件名，确认后将当前编辑的文件写入磁盘。
	* 退出系统（Quit）：退出前检查是否有未保存的编辑内容，若需要则执行Save操作后再退出。
2. 文本窗口编辑功能（EDIT）
	* 插入字符：定位光标，在光标处之后插入字符，每插入一个字符后光标定位在新插入的字符之后。
	* 插入行：插入字符为回车时，光标后内容为新行。
	* 删除字符：定位光标，“Delete”键向后删除字符，“Backspace”键向前删除字符。
	* 删除行：光标位于行首，输入“Backspace”键
	* 查找字符/串：提示用户输入要查找的字符串，从当前光标处向后定位，找到时光标置于首字符之前。
	* 替换字符/串：提示用户输入原字符串和新字符串，从当前光标处向后定位，找到时光标置于首字符前，由用户对是否替换进行确认。
	* 块操作（选择）：定位块首、块尾，块拷贝、块删除。
3. 选做：撤销操作
## 所用工具环境配置
使用系统：Windows10 & Windows 11

使用语言：C++

编译器：g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

编译命令：g++ .\cursorPos.cpp .\fileContent.cpp .\header.h .\main.cpp .\vim_r.cpp .\environment.cpp -o SimpleWord
## 使用的数据结构
链表、数组（vector）、字符串（string）
## 分析与实现
在Linux中，有一个非常著名的编辑器，叫做Vim。其特点在于，只依赖键盘即可高效地完成一系列的编辑操作。

受到Vim的启发，我们决定仿照它来实现一个在Windows的命令行下运行的文本编辑器

### 文件概览
* header.h，各个类的声明，名为xxx的类的具体实现见xxx.cpp
* main.cpp，主函数入口，负责创建类的实体和运行
* vim_r.cpp，编辑器类，编辑器的本体
* fileContent.cpp，文件内容类，用于记录编辑器缓冲区内的内容，其实质为双向链表
* cursorPos.cpp，光标类，用来记录当前光标的位置
* environment.cpp，环境类，用来记录每一步的缓冲区内容及相关信息，用于撤销操作

### 类成员详解
1. vim_r
	* filename，string类型变量，记录当前文件的文件名
	* ft，fileContent *类型变量，记录当前缓冲区链表头节点地址
	* clipBoard，fileContent *类型变量，记录当前剪贴板链表头节点地址
	* cp，cursorPos类型变量，记录当前可移动光标的位置
	* visualCursor，cursorPos类型变量，记录visual模式下固定光标的位置
	* m，mode类型变量，记录当前编辑器模式
	* changed，bool类型变量，记录当前缓冲区是否有过修改
	* currentCursorAhead，bool类型变量，记录visual模式下可移动光标是否在固定光标之前
	* message，string类型变量，记录编辑器最下方展示的信息内容
	* lastSearch，string类型变量，记录上次搜索的方向和目标
	* historyEnvironment，vector类型变量，记录过去的缓冲区内容及相关信息
	* currentEnvironmentIndex，int类型变量，记录当前缓冲区在historyEnvironment中的位置
	* hOutBuffer，HANDLE *类型变量，记录两个屏幕缓冲区（并非文件缓冲区）
	* displayCount，int类型变量，记录当前使用的屏幕缓冲区索引，控制光标的显示情况
	* vim_r(char *filename)，构造函数，进行变量初始化及将文件读入缓冲区
	* run()，调用display展示文件缓冲区的内容，同时监听键盘输入
	* takeAction(int ch)，对键盘输入进行初步处理和分流
	* takeActionNormal(int ch)，对normal模式下的键盘输入进行响应
	* takeActionInsert(int ch)，对insert模式下的键盘输入进行响应
	* takeActionVisual(int ch)，对visual模式下的键盘输入进行响应
	* takeActionCommand(int ch)，对command模式下的键盘输入进行响应
	* takeActionCommand(string message)，对command模式下的输入的完整命令进行响应
	* takeActionReplace(int ch)，对Replace模式下的键盘输入进行响应
	* display(int activeBuffer)，将当前文件缓冲区内容输出至屏幕缓冲区
	* loadEnvironment(int index)，加载存储于historyEnvironment[index]的环境
	* saveEnvironment()，保存当前环境
2. fileContent
	* line，string类型变量，记录当前节点所包含的逻辑行内容（并非屏幕行）
	* prev，fileContent *类型变量，指向当前节点的上一个逻辑行的节点
	* next，fileContent *类型变量，指向当前节点的下一个逻辑行的节点
	* fileContent(string line)，构造函数，进行变量初始化
	* emptyLine()，返回当前行是否为空行
3. cursorPos
	* linePos，fileContent *类型变量，记录当前光标的行的节点
	* charPos，int类型变量，记录当前光标的列数
	* cursorPos()，构造函数，初始化变量
	* cursorPos(const cursorPos &c)，拷贝构造函数
	* moveCursor(mode m, direction d)，在模式m下朝d的方向移动光标
4. environment
	* ft，fileContent *类型变量，指向记录过去缓冲区内容的链表头
	* changed，bool类型变量，记录过去缓冲区是否被修改
	* environment()，构造函数，初始化变量
	* environment(const environment &c)，拷贝构造函数
	* ~environment()，析构函数，释放ft指向的链表的内存

### 实现功能汇总
1. 启动
	* 在cmd中输入“SimpleWord.exe”可创建一个空缓冲区（并没有创建文件）
	* 在cmd中输入“SimpleWord.exe xxx”可打开一个已有的文件，其中xxx为文件的路径。如果文件不存在，会展示相应提示且创建一个空缓冲区
2. normal模式
	* j，移动光标至下一个逻辑行的相同列处
	* k，移动光标至上一个逻辑行的相同列处
	* l，在同一个逻辑行中向右移动光标
	* h，在同一个逻辑行中向左移动光标
	* 0，移动光标至逻辑行行首
	* \$，移动光标至逻辑行行尾
	* G，移动光标至文件缓冲区最后一行行首
	* x，删除当前光标下字符，并将被删除的字符写入剪贴板
	* ~，如果当前光标下字符为字母的话，改变其大小写
	* p，将当前剪贴板内容粘贴至当前光标下字符后
	* i，进入插入模式，光标位置不变
	* a，进入插入模式，光标向右移动一位
	* o，在当前光标所在行下面创建一个空行，将光标置于新行行首，进入insert模式
	* O，在当前光标所在行上面创建一个空行，将光标置于新行行首，进入insert模式
	* R，进入replace模式，光标位置不变
	* v，进入visual模式，光标位置不变
	* :，进入command模式，后续可输入命令
	* /，进入command模式，后续可输入正向搜索的内容
	* ?，进入command模式，后续可输入反向搜索的内容
	* n，重复上次搜索，方向与上次相同
	* N，重复上次搜索，方向与上次相反
	* u，撤销上次修改
	* \<C-r\>，取消上次撤销
	* \<del\>，同x
3. insert模式
	* \<CR\>，在当前光标所在行下面创建一个空行，将光标置于新行行首
	* \<BS\>，删除当前光标前面的字符
	* \<esc\>，回到normal模式
	* \<del\>，删除当前光标下面的字符
	* 其它可见字符，将相应字符插入至当前光标下，光标向后移一位
4. command模式
	* \<CR\>，执行目前输入的命令
	* \<BS\>，删除输入的最后一个字符
	* \<esc\>，回到normal模式
	* 其它可见字符，添加至命令末尾
	* :w，保存当前文件缓冲区到文件。如果当前文件不存在会有相应提示
	* :write，同:w
	* :w xxx，保存当前文件缓冲区至xxx，其中xxx为一文件名
	* :write xxx，同:w xxx
	* :q，退出编辑器，如果当前文件缓冲区未保存会有相应提示
	* :quit，同:q
	* :q!，退出编辑器，不保存当前文件缓冲区
	* :wq，保存当前文件缓冲区并退出
	* :s/[s1]/[s2]/[flag]，将字符串s1替换为s2，flag有以下选项：
		* c，逐个确认是否进行替换
		* g，替换每行中的所有匹配，默认为替换每行的第一个匹配
	* /[s]，从当前光标向后查找字符串s，跳转到第一个匹配，查找到文件尾自动从文件头查找，且有相应提示；全文件均未查找到，也有相应提示
	* ?[s]，从当前光标向前查找字符串s，跳转到第一个匹配，查找到文件头自动从文件尾查找，且有相应提示；全文件均未查找到，也有相应提示
5. visual模式
	* j，移动可移动光标至下一个逻辑行的相同列处
	* k，移动可移动光标至上一个逻辑行的相同列处
	* l，在同一个逻辑行中向右移动可移动光标
	* h，在同一个逻辑行中向左移动可移动光标
	* d，删除可移动光标和固定光标之间的内容，包括光标下字符，并将被删除的字符写入剪贴板
	* x，同d
	* y，复制可移动光标和固定光标之间的内容至剪贴板，包括光标下字符
	* \<esc\>，回到normal模式
	* \<del\>，同d
6. replace模式
	* \<BS\>，在同一个逻辑行中向左移动光标
	* \<esc\>，回到normal模式
	* \<del\>，删除当前光标下面的字符
	* 其它可见字符，将当前光标下的字符替换为输入字符，并将光标向右移一位
## 测试与结论
