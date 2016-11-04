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

// The terminal prompt elements
char machineName[HOST_NAME_MAX];
char currentDirectory[PATH_MAX];
char * userName;
char prompt[HOST_NAME_MAX+PATH_MAX+1000];
// Storing the shell process ID
int parentID;
// int foreground process ID
int forgndProc;
bool bkgndProc = false;


/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
string rl_gets ()
{
	// read line 
	char *line_read = (char *)NULL;
	/* Get a line from the user. */
	// prompt
	//string s = "r" + "wr";
	//char * s = "roger";
 	line_read = readline (prompt);

	/* If the line has any text in it, save it on the history. */
	if (line_read && *line_read)
    	add_history (line_read);

	string line(line_read);
	free(line_read);
	return (line);
}

/**
 * Called when Ctrl+C signal is received
 * Exit the child process if it exists
 * Prints a new line in the parent process
 */
void ctrlCHandler(int s){

	// The parent process
	if (getpid() == parentID){
		int child_status, wait_ret;
		// wait for the foreground process child
		wait_ret = waitpid(forgndProc,&child_status,WUNTRACED);
		// If the parent does not have a child in the foreground, print the prompt line
		if(wait_ret==-1){
			printf("\n%s@%s:%s$ ", userName, machineName,currentDirectory);
	    	fflush(stdout);
	    }
	    else // If the parent already has a foreground child, enter a new line
	    	printf("\n");
	}
}

/**
 * Reads a line from user and returns it. The user needs to
 * press "Enter" to stop inputting
 *
 * @return the input line.
 */
string readLine() {
	// Display the terminal prompt line
	//printf("%s@%s:%s$ ", userName, machineName,currentDirectory);

	// Read the command
	string inputLine = rl_gets();
	//getline(cin,inputLine);

	return inputLine;
}

/**
 * Reutrns an array of the sent string splitted into words
 *
 * @param		line	the command and all of its arguments in one string
 * @return 				the array of words
 */
char **splitLine(string line) {
	if (*line.rbegin() == '&') {
		*line.rbegin() = '\0';
		bkgndProc = true;
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
	args_ch = new char*[args.size()]; // allocating memory for number of args
	char * tmp_ch;
	for(int i = 0; i < args.size(); i++)
	{
		args_ch[i] = new char[args[i].size()];
		tmp_ch = new char[args[i].size()]; // allocating memory for the size of each arg
		strcpy(tmp_ch,args[i].c_str());
		args_ch[i] = tmp_ch;
	}
	args_ch[args.size()] = NULL; // necessary to terminate the char ** with a null for correct functionality of execvp
	return args_ch;
}

/**
 * Forks the process and executes the passed command in args[0]
 *
 * @param		args	an array of arrays of characters where args[0] is
 *								the desired command to be run and the rest are the
 *								different arguments with the final argument being
 *								a null terminator
 */
void launchCommand(char **args) {
	// Parse and execute
	int pid, stat_loc;
	pid = fork();
	if (pid == -1) {
		perror("error in fork");
		//handling of the child
	} else if (pid == 0) {
		// Set the background process in a new session to avoid Ctrl+C signal affecting them
		if (bkgndProc)
			setsid();
		// Execute the command in a new process using execvp
		if (execvp(args[0], args) == -1) {
				printf("%s: command not found\n", args[0]);
		}
		return;
		//handling of the parent
	} else {
		if (bkgndProc) {
			//add to history
		} else {
			forgndProc = pid;
			pid = wait(&stat_loc);
			if (!(stat_loc & 0x00FF)) {}
		}
	}
}

/**TODO write documentation*/
int changeDirectory(char **args) {
	if (args[1] == NULL || strcmp(args[1], "~") == 0) {
		chdir(getenv("HOME"));
	} else if (chdir(args[1]) != 0) {
		//TODO convert args[1]
		// perror("Error in change directory: %s\n",args[1]);
		perror("Error in change directory:");
	}
	getcwd(currentDirectory,PATH_MAX);
	return 1;
}

/**TODO write documentation*/
void executeCommand(char **args) {
	//TODO handle the case where there is no arguments
	if (strcmp(args[0], "cd") == 0) {
		changeDirectory(args);
	} else {
		launchCommand(args);
	}
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
		executeCommand(args);

	} while(status);
}

int main(int argc, char ** argv) {
	// initialization of the terminal prompt elements
	userName = getlogin();
	getcwd(currentDirectory,PATH_MAX);
	gethostname(machineName,HOST_NAME_MAX);
	strcpy(prompt, "");
	strcat(prompt,userName);
	strcat(prompt,"@");
	strcat(prompt,machineName);
	strcat(prompt,":");
	strcat(prompt,currentDirectory);
	strcat(prompt,"$ ");

	//"\n%s@%s:%s$ ", userName, machineName,currentDirectory)

	parentID = getpid();
	// Handling Ctrl+C
	signal (SIGINT,ctrlCHandler);
	mainloop();
	return 0;
}
