# data_structure_homework1
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
