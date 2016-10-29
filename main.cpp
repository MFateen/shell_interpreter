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

void mainloop() {
	int status = 1; // Status of the shell
	do {
		// Display the default line while waiting for input command
		char * userName = getlogin();
		char machineName[HOST_NAME_MAX];
		char currentDirectory[PATH_MAX];
		getcwd(currentDirectory,PATH_MAX);
		gethostname(machineName,HOST_NAME_MAX);

		printf("%s@%s:%s$", userName, machineName,currentDirectory);

		// Read the command
		string command;
		getline(cin,command);

		// Split the command line to its elements: command, options, arguments (separated by space)
		char delimiter = ' ';
		stringstream ss;
		ss.str(command);
		string tmpString;
		vector<string> commandElements;

		while(getline(ss,tmpString,delimiter)) {
			commandElements.push_back(tmpString);
		}

		for (int i = 0 ; i < commandElements.size(); i++)
			printf("%s\n",commandElements[i].c_str() );

		// Parse and execute
		int pid, stat_loc;
		pid = fork();
		if (pid == -1) {
			perror("error in fork");
			//handling of the child
		} else if (pid == 0) {
			printf("This is the child op\n");
			return;
			//handling of the parent
		} else {
			pid = wait(&stat_loc);
			if (!(stat_loc & 0x00FF)) {
				printf("the child terminated\n");
			}
		}
		// A built-in command

		// A process

	} while(status);
}

int main(int argc, char ** argv) {
	mainloop();
	return 0;
}
