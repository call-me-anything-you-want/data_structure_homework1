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

编译器：gcc (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0
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
## 测试与结论
