/**
 * @file main.cpp
 * @date October 28, 2016
 * @author Abdallah sobehy
 * @author Mostafa Fateen
 * @brief main for the shell interpreter
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
int main(int argc, char ** argv)
{

	int status = 1; // Status of the shell
	while(status)
	{
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
		while(getline(ss,tmpString,delimiter))
		{
			commandElements.push_back(tmpString);
		}
		// for (int i = 0 ; i < commandElements.size(); i++)
		// 	printf("%s\n",commandElements[i].c_str() );

		// Parse and execute

			// A built-in command

			// A process

	}
	return 0;
}