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
## 使用的数据结构
## 分析与实现
## 测试与结论
this repo is trying to design an editor that works under windows terminal.

we hope to realize some certain functions of the editor vim.

plan:
* create a file (like typing "vim" in cmd)
* open a file (like typing "vim xxx.txt" in cmd)
* save a file (like ":w" in vim)
* quit (like ":q" in vim)
* insert character
* insert "enter"
* delete (using "Del" and "BS")
* delete (delete the whole line using "Del" at the head of the line, we can try "dd" in vim as well)
* substitute (needs to be comfirmed by the user)
* block operation
* undo and redo (using stack)

======================================================

vim_r is the editor class (short for vim_reduced).

ft is used to save the current content displayed.

m is used to save the current mode.

run() is the function to show the content and listen to the keyboard.

takeAction() is the function to take different actions according the input char.
