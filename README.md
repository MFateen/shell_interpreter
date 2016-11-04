# **Shell Interpreter**


Shell is a utility program with the Linux system that serves as an interface between the user and the kernel and also plays a very important rule as a Command Interpreter and should be able to do the following :

* Reads the command

* Locates the file in the directories containing utilities

* Loads the utility into memory

* Executes the utility as a child of its own

## Dependencies
- libreadline6
- libreadline6-dev

## Run the shell interpreter
1. sudo apt-get install libreadline6 libreadline6-dev
2. make
3. ./main.o

## Requirements
- [x] Accept a Linux command and execute it when the return key is pressed.
- [x] Accept CTRL+C to print “^C” then close the current process executing in the shell.
- [x] Implement the ampersand control operator "*&*".
- [x] An error message should be printed on entering unsupported command.
- [x] Use the up arrow only on your shell to retrieve the history of the commands​.
- [x] Implement correct TAB completion for file and folder names only.

## Authors
- Abdallah *SOBEHY* ([Abdallah-Sobehy] (https://github.com/Abdallah-Sobehy))
- Mostafa *FATEEN* ([MFateen] (https://github.com/MFateen))
