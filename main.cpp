/**
 * @file main.cpp
 * @date October 28, 2016
 * @author Abdallah sobehy
 * @author Mostafa Fateen
 * @brief main for the shell interpreter
 * @ref https://github.com/brenns10/lsh/blob/master/src/main.cd
 * @ref http://web.mit.edu/gnu/doc/html/rlman_2.html
 */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
// Maximum length of a user name
#define USER_NAME_MAX 100

// The terminal prompt elements
char prompt[HOST_NAME_MAX+PATH_MAX+USER_NAME_MAX];

// int foreground process ID
int frgndPID;
// Flag for background process
bool bkgndProc = false;

// Two complementary functions
void printTop();
void aboutus();

/**
 * Generates and updates the prompt line that looks like:
 * <yourName>@<​machineName>​:<path>$
 */
void updatePrompt() {
	char machineName[HOST_NAME_MAX];
	char currentDirectory[PATH_MAX];
	char * userName;
	userName = getlogin();
	getcwd(currentDirectory,PATH_MAX);
	gethostname(machineName,HOST_NAME_MAX);
	strcpy(prompt, "");
	// Write in green
	strcat(prompt,"\033[1;32m");
	strcat(prompt,userName);
	strcat(prompt,"@");
	strcat(prompt,machineName);
	strcat(prompt,"\033[0m");
	strcat(prompt,":");
	// Write in blue
	strcat(prompt,"\033[1;34m");
	strcat(prompt,currentDirectory);
	strcat(prompt,"\033[0m");
	strcat(prompt,"$ ");
}

/**
 * Reads input from stdin allowing history and tab completion. It also sets the
 * global boolean bkgndProc to true if there was an '&' at the end of the input
 *
 * @return the input string
 * @see http://web.mit.edu/gnu/doc/html/rlman_2.html
 */
string readLine () {
	// read line
	char *line_read = (char *)NULL;
	// read input while shwing the prompt line
 	line_read = readline (prompt);

	/* If the line has any text in it, save it on the history. */
	if (line_read && *line_read)
    	add_history (line_read);

	string line(line_read);
	free(line_read);

	if (*line.rbegin() == '&') {
		*line.rbegin() = '\0';
		bkgndProc = true;
	}

	return line;
}

/**
 * Called in the parent process when Ctrl+C signal is received
 *
 * Prints ^C and a new line in the parent process
 * If there was a **foreground** child process it will exit
 */
void ctrlCHandler(int s) {
	// When the ctrl+c is pressed an INT signal is sent to all the processes
	// that have the same PGID (Process Group ID). If the signal has a handler
	// other than the default, it will run such as the case in the parent.
	// However, if it only has the default handler it will stop.

	// note: that the PGID of the background processes is changed on their
	// creation so that they don't receive that signal

	int child_status, wait_ret;
	// wait for the foreground process child
	wait_ret = waitpid(frgndPID,&child_status,WUNTRACED);
	// If the parent does not have a child in the foreground, print the prompt line
	if(wait_ret==-1){
		printf("\n%s",prompt);
    fflush(stdout);
  }
	else { // If the parent already has a foreground child, enter a new line
  	printf("\n");
	}
}


/**
 * Takes a string of a command and its arguments and split it into
 * an array of arguments where args[0] is the command.
 *
 * @param   line the command and all of its arguments in one string
 * @return       the array of arguments
 */
char **splitLine(string line) {
	if (line.empty()) {
		return NULL;
	}

	char delimiter = ' ';
	stringstream ss;
	ss.str(line);
	string tmpString;
	vector<string> args;

	while(getline(ss,tmpString,delimiter)) {
		args.push_back(tmpString);
	}

	// storing args as char* for execvp function
	char ** args_ch ;
	// allocating memory for number of args
	args_ch = new char*[args.size()];
	char * tmp_ch;

	for(int i = 0; i < args.size(); i++) {
		args_ch[i] = new char[args[i].size()];
		// allocating memory for the size of each arg
		tmp_ch = new char[args[i].size()];
		strcpy(tmp_ch,args[i].c_str());
		args_ch[i] = tmp_ch;
	}

	// terminate the char ** with a null for the execvp()
	args_ch[args.size()] = NULL;
	return args_ch;
}

/**
 * Forks the process and executes the passed command in args[0]
 *
 * @param args an array of arrays of characters where args[0] is the desired
 *             command to be run and the rest are the different arguments with
 *             the final argument being a null terminator "\0".
 */
void launchCommand(char **args) {
	int pid, stat_loc;
	pid = fork();
	if (pid == -1) {
		perror("error in fork");
		//handling of the child
	}
	else if (pid == 0) {
		// Set the background process in a new session to avoid Ctrl+C signal affecting them
		if (bkgndProc) {
			setsid();
		}

		// Execute the command in a new process using execvp
		if (execvp(args[0], args) == -1) {
			printf("%s: command not found\n", args[0]);
		}

		return;
	}
	//handling of the parent
	else {
		if (!bkgndProc){
			frgndPID = pid;
			pid = wait(&stat_loc);
		}
	}
}

/**
 * Changes the shell working directory to the passed one, with a default
 * behaviour of changing the directory to the HOME if no arguments were passed
 *
 * if args[1] = NULL or ~    : change directory to the HOME
 *            = ..           : change directory to parent directory
 *            = .            : stay at the same directory
 *            = path/to/dir  : go to the designated directory
 *
 * @param args an array of arrays of characters where args[1] is the path to
 *             the desired directory and the final argument being a "\0".
 */
void changeDirectory(char **args) {
	// Change directory to HOME
	if (args[1] == NULL || strcmp(args[1], "~") == 0) {
		chdir(getenv("HOME"));
	}
	else if (chdir(args[1]) != 0) {
		printf("Error in change directory: %s\n",args[1]);
	}

	// Update prompt to show the new path
	updatePrompt();
}

/**
 * Checks if the the command in args[0] is predefined. If so, it will Called
 * the appropriate function. Otherwise, launchCommand is called with the
 * exact parameter.
 *
 * The predefined commands:
 *     cd    : changes the working directory
 *     about : prints some basic info about the project and its authors
 *     quit  : exits the program
 *
 * @param args an array of arrays of characters where args[0] is the desired
 *             command to be run and the rest are the different arguments with
 *             the final argument being a null terminator "\0".
 *
 * @return     the status of the program 1 to keep running and 0 to quit
 */
int executeCommand(char **args) {
	if (args == NULL) {
		return 1;
	}
	if (strcmp(args[0], "cd") == 0) {
		changeDirectory(args);
	}
	else if (strcmp(args[0], "quit") == 0) {
		return 0;
	}
	else if (strcmp(args[0], "about") == 0) {
		aboutus();
	}
	else {
		launchCommand(args);
	}
	return 1;
}

/**
 * The core of the shell. It reads the command from the user,
 * splits the user input, and executes the command
 */
void mainloop() {
	int status = 1; // Status of the shell
	do {
		bkgndProc = false;
		// Read the command
		string line = readLine();

		// Split the command line to its elements: command, options, arguments (separated by space)
		char** args = splitLine(line);

		// Executes the input command with the arguments
		status = executeCommand(args);

	} while(status);
}

int main(int argc, char ** argv) {
	// Handling Ctrl+C
	signal (SIGINT,ctrlCHandler);

	printTop();

	// initialization of the terminal prompt elements
	updatePrompt();

	mainloop();

	return 0;
}

void aboutus() {
	system("clear");
	printf("This is a shell interpreter implemented in C++ as a part of the first\n");
	printf("Assignment for the Operating System Module of the Computer Engineering\n");
	printf("Departement in Cairo University\n");
	printf("\n");
	printf("This is the work of both:\n\t* Abdallah Sobehy (http://github.com/Abdallah-Sobehy)\n\t* Mostafa Fateen (http://github.com/MFateen)\n");
	printf("\n");
	printf("The source code along with the README.md could be seen on github\n");
	printf("via this link https://github.com/Abdallah-Sobehy/shell_interpreter\n");
	printf("\n");
	printf("References and useful links:\n");
	printf("\t* https://github.com/brenns10/lsh/blob/master/src/main.cd\n");
	printf("\t* http://web.mit.edu/gnu/doc/html/rlman_2.htmln");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
}

/**
 * A function that prints a header of the authors' name and a title.
 */
void printTop() {
	system("clear");
	printf("      \x1B[33m  ___       _                    \x1B[0m  __\x1B[36m ___       _          _         \x1B[0m\n");
	printf("      \x1B[33m | __|__ _ | |_  ___  ___  _ _   \x1B[0m / /\x1B[36m/ __| ___ | |__  ___ | |_  _  _ \x1B[0m\n");
	printf("      \x1B[33m | _|/ _` ||  _|/ -_)/ -_)| ' \\ \x1B[0m / /\x1B[36m \\__ \\/ _ \\| '_ \\/ -_)| ' \\| || |\x1B[0m\n");
	printf("      \x1B[33m |_| \\__,_| \\__|\\___|\\___||_||_|\x1B[0m/_/\x1B[36m  |___/\\___/|_.__/\\___||_||_|\\_, |\x1B[0m\n");
	printf("                                                                      \x1B[36m|__/ \x1B[0m\n");
	printf(" +-+-+ +-+-+-+-+-+-+-+-+-+-+ +-+-+-+ +-+-+-+ +-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+\n");
	printf(" |A|n| |A|s|s|i|g|n|m|e|n|t| |f|o|r| |t|h|e| |O|p|e|r|a|t|i|n|g| |S|y|s|t|e|m|s|\n");
	printf(" +-+-+ +-+-+-+-+-+-+-+-+-+-+ +|m|o|d|u|l|e|+ +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+\n");
	printf("                              +-+-+-+-+-+-+\n");
	printf("\n\n\n\n");
	printf("\nFor more info please enter the command   \"\033[1;31mabout\033[0m\"\n");
	printf("and to exit type                         \"\033[1;31mquit\033[0m\"\n\n");

}
