/**
 * @file main.cpp
 * @date October 28, 2016
 * @author Abdallah sobehy
 * @author Mostafa Fateen
 * @brief main for the shell interpreter
 * @ref https://github.com/brenns10/lsh/blob/master/src/main.cd
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

using namespace std;

// The terminal prompt elements
char machineName[HOST_NAME_MAX];
char currentDirectory[PATH_MAX];
char * userName;

/**
 * Reads a line from user and returns it. The user needs to
 * press "Enter" to stop inputting
 *
 * @return the input line.
 */
string readLine() {
	// Display the terminal prompt line
	printf("%s@%s:%s$", userName, machineName,currentDirectory);

	// Read the command
	string inputLine;
	getline(cin,inputLine);

	return inputLine;
}

/**
 * Reutrns an array of the sent string splitted into words
 *
 * @param		line	the command and all of its arguments in one string
 * @return 				the array of words
 */
char **splitLine(string line) {
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
		// Execute the command in a new process using execvp
		if (execvp(args[0], args) == -1) {
				perror("execvp error!");
		}
		return;
		//handling of the parent
	} else {
		pid = wait(&stat_loc);
		if (!(stat_loc & 0x00FF)) {
			//printf("the child terminated\n");
		}
	}
}

/**TODO write documentation*/
int changeDirectory(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "cd command expects arguments");
	} else if (chdir(args[1]) != 0) {
			perror("Error in change directory");
	}
	return 1;
}

/**TODO write documentation*/
void executeCommand(char **args) {
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

	mainloop();
	return 0;
}
