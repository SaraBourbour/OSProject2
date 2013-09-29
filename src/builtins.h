#pragma once
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

// Readline lib
#include <readline/readline.h>
#include <readline/history.h>

// Return codes for builtins
#define NORMAL_EXIT         int(0)
#define BAD_FILE_OR_DIR     int(1)
#define INVALID_ARGUMENTS   int(2)
#define ABNORMAL_EXEC		int(3)
#define TOO_MANY_ARGUMENTS	int(4)
#define NOT_READY			int(5)
// Signals for exiting shell
#define SIGNAL_EXIT_SHELL   int(-1)

// Filename for history persistence
#define HISTORY_FILE		"~/.h_history"

using std::vector;
using std::string;


// Lists all the files in the specified directory. If not given an argument,
// the current working directory is used instead.
int com_ls(vector<string>& tokens);


// Changes the current working directory to that specified by the given
// argument.
int com_cd(vector<string>& tokens);


// Displays the current working directory.
int com_pwd(vector<string>& tokens);


// If called without an argument, then any existing aliases are displayed.
// Otherwise, the second argument is assumed to be a new alias and an entry
// is made in the alias map.
int com_alias(vector<string>& tokens);


// Removes aliases. If "-a" is provided as the second argument, then all
// existing aliases are removed. Otherwise, the second argument is assumed to
// be a specific alias to remove and if it exists, that alias is deleted.
int com_unalias(vector<string>& tokens);


// Prints all arguments to the terminal.
int com_echo(vector<string>& tokens);


// Exits the program.
int com_exit(vector<string>& tokens);


// Displays all previously entered commands, as well as their associated line
// numbers in history.
int com_history(vector<string>& tokens);


// Returns the current working directory.
string pwd();

// Returns the current user.
string user();

string last_command_status(int code);

