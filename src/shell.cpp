#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

#include "builtins.h"

// Potentially useful #includes (either here or in builtins.h):
//   #include <dirent.h>
//   #include <errno.h>
//   #include <fcntl.h>
//   #include <signal.h>
//   #include <sys/errno.h>
//   #include <sys/param.h>
//   #include <sys/types.h>
//   #include <sys/wait.h>
//   #include <unistd.h>

using namespace std;


// The characters that readline will use to delimit words
const char* const WORD_DELIMITERS = " \t\n\"\\'`@><=;|&{(";

// An external reference to the execution environment
extern char** environ;

// Define 'command' as a type for built-in commands
typedef int (*command)(vector<string>&);

// A mapping of internal commands to their corresponding functions
map<string, command> builtins;

// Variables local to the shell
map<string, string> localvars;

// A list of accepted redirect operators
string redirect_operators[] = {"<", ">", ">>"};

// Tracks if there are redirects that need to be processed for this line
bool std_in_redirect = false;
bool std_out_redirect = false;
bool std_out_append_redirect = false;



// Handles external commands, redirects, and pipes.
int execute_external_command(vector<string> tokens) {
	int child_PID = -1;
	int child_return_code = -1;
	// Fork off a new process
	d_cout("Forking a child process\n");
	child_PID = fork();
	// If this process is the child
	if (child_PID == 0) {
		d_cout("Inside the child process\n");
		// Generate the command char *const command[], with NULL on the end
		char *command[tokens.size() + 1];
		d_cout("Generating command array\n");
		for (int i = 0; i < tokens.size(); i++) {
			// Create an array to hold the string
			char tmp_c_string[tokens[i].size() + 1];
			// Copy the string over, using sketchy string copy
			size_t length_of_temp = tokens[i].copy(tmp_c_string, tokens[i].size(), 0);
			// Null terminate the string
			tmp_c_string[length_of_temp]='\0';
			// Set the string to the args array index
			command[i] = tmp_c_string;
			d_cout("Setting next command index to ", command[i], "\n");
		}
		// Don't forget the null
		command[tokens.size() + 1] = NULL;
		
		// Try execution with each path in $PATH
		d_cout("Preparing to path match the command\n");
		string path = getenv("PATH");
		string path_delimiter = ":";
		size_t current_position = 0;
		string path_component;
		bool executed = false;
		string file_to_execute;
		while ((current_position = path.find(path_delimiter)) != string::npos && !executed) {
			path_component = path.substr(0, current_position);
			d_cout("Path component: ", path_component.c_str());
			// Append the path component to the command
			file_to_execute = path_component + command[0];
			d_cout("Trying command ", file_to_execute.c_str(), "\n");
			// Attempt to execute it
			int ret_val = execve(file_to_execute.c_str(), command, environ);
			// If the execution was good
			if (ret_val != EXEC_FAIL) {
				// Mark executed so we stop
				executed = true;
				// Return normal exit code
				return NORMAL_EXIT;
			}
			// Chop that part of the path string off
			path.erase(0, current_position + path_delimiter.length());
		}
		// Try execution with PWD for the command if PATH didn't find it
		if (!executed) {
			file_to_execute = pwd() + command[0];
			int ret_val = execve(file_to_execute.c_str(), command, environ);
			if (ret_val != EXEC_FAIL) {
				executed = true;
				return NORMAL_EXIT;
			}
		}
		
		// If we still haven't executed, return command not found
		return CMD_NOT_FOUND;
	}
	// Else this is the parent
	else {
		d_cout("In parent process\n");
		// Wait for the child to exit
		wait(&child_return_code);
		d_cout("Child exited, resuming parent control\n");
	}
	
	// Check return codes for the external command
    return NORMAL_EXIT;
}


// Return a string representing the prompt to display to the user. It needs to
// include the current working directory and should also use the return value to
// indicate the result (success or failure) of the last command.
string get_prompt(int return_value) {
    string prompt = "";
    prompt = user() + " | " + pwd() + " | " + last_command_status(return_value) + "\n   % ";
    return prompt;
}


// Return one of the matches, or NULL if there are no more.
char* pop_match(vector<string>& matches) {
    if (matches.size() > 0) {
        const char* match = matches.back().c_str();
        
        // Delete the last element
        matches.pop_back();
        
        // We need to return a copy, because readline deallocates when done
        char* copy = (char*) malloc(strlen(match) + 1);
        strcpy(copy, match);
        
        return copy;
    }
    
    // No more matches
    return NULL;
}


// Generates environment variables for readline completion. This function will
// be called multiple times by readline and will return a single cstring each
// time.
char* environment_completion_generator(const char* text, int state) {
    // A list of all the matches;
    // Must be static because this function is called repeatedly
    static vector<string> matches;
    
    // If this is the first time called, construct the matches list with
    // all possible matches
    if (state == 0) {
        // TODO: YOUR CODE GOES HERE
    }
    
    // Return a single match (one for each time the function is called)
    return pop_match(matches);
}


// Generates commands for readline completion. This function will be called
// multiple times by readline and will return a single cstring each time.
char* command_completion_generator(const char* text, int state) {
    // A list of all the matches;
    // Must be static because this function is called repeatedly
    static vector<string> matches;
    
    // If this is the first time called, construct the matches list with
    // all possible matches
    if (state == 0) {
        // TODO: YOUR CODE GOES HERE
    }
    
    // Return a single match (one for each time the function is called)
    return pop_match(matches);
}


// This is the function we registered as rl_attempted_completion_function. It
// attempts to complete with a command, variable name, or filename.
char** word_completion(const char* text, int start, int end) {
    char** matches = NULL;
    
    if (start == 0) {
        rl_completion_append_character = ' ';
        matches = rl_completion_matches(text, command_completion_generator);
    } else if (text[0] == '$') {
        rl_completion_append_character = ' ';
        matches = rl_completion_matches(text, environment_completion_generator);
    } else {
        rl_completion_append_character = '\0';
        // We get directory matches for free (thanks, readline!)
    }
    
    return matches;
}


// Transform a C-style string into a C++ vector of string tokens, delimited by
// whitespace.
vector<string> tokenize(const char* line) {
    vector<string> tokens;
    string token;
    
    // istringstream allows us to treat the string like a stream
    istringstream token_stream(line);
    d_cout("Tokenizing line\n");
    while (token_stream >> token) {
		d_cout("Pushing back ", token.c_str(), "\n");
        tokens.push_back(token);
    }
    
    // Search for quotation marks, which are explicitly disallowed
    for (size_t i = 0; i < tokens.size(); i++) {
        
        if (tokens[i].find_first_of("\"'`") != string::npos) {
            cerr << "\", ', and ` characters are not allowed." << endl;
            
            tokens.clear();
        }
    }
    
	d_cout("Tokenizing complete\n");
    return tokens;
}


// Executes a line of input by either calling execute_external_command or
// directly invoking the built-in command.
int execute_line(vector<string>& tokens, map<string, command>& builtins) {
    int return_value = 0;
    
    if (tokens.size() != 0) {
        map<string, command>::iterator cmd = builtins.find(tokens[0]);
        
        if (cmd == builtins.end()) {
			d_cout("Could not find an internal command, trying external\n");
            return execute_external_command(tokens);
        } else {
            return ((*cmd->second)(tokens));
        }
    }
    return BLANK_COMMAND;
}


// Substitutes any tokens that start with a $ with their appropriate value, or
// with an empty string if no match is found.
void variable_substitution(vector<string>& tokens) {
    vector<string>::iterator token;
    
    for (token = tokens.begin(); token != tokens.end(); ++token) {
        
        if (token->at(0) == '$') {
            string var_name = token->substr(1);
            
            if (getenv(var_name.c_str()) != NULL) {
                *token = getenv(var_name.c_str());
            } else if (localvars.find(var_name) != localvars.end()) {
                *token = localvars.find(var_name)->second;
            } else {
                *token = "";
            }
        }
		// Replace ~ with home
		else if (token->at(0) == '~') {
			*token = getenv("HOME");
		}
	}
}


// Examines each token and sets an env variable for any that are in the form
// of key=value.
void local_variable_assignment(vector<string>& tokens) {
    vector<string>::iterator token = tokens.begin();
    
    while (token != tokens.end()) {
        string::size_type eq_pos = token->find("=");
        
        // If there is an equal sign in the token, assume the token is var=value
        if (eq_pos != string::npos) {
            string name = token->substr(0, eq_pos);
            string value = token->substr(eq_pos + 1);
            
            localvars[name] = value;
            
            token = tokens.erase(token);
        } else {
            ++token;
        }
    }
}

string history_substitution(string token) {
	d_cout("Beginning history substitution for token: ", token.c_str(), "\n");
	char** new_tokens = new char*[1];
	int ret_val = history_expand((char*)token.c_str(), new_tokens);
	d_cout("Expansion complete. Return code: ", ret_val, "\n");
	
	if (ret_val != NORMAL_EXIT_EXPANSION) {
		d_cout("Token: ", token.c_str(), " not substituted\n");
		return token;
	}
	
	stringstream returnString;
	returnString << new_tokens[0];
	
	return returnString.str();
}

void initializeShell() {
	// Populate the map of available built-in functions
    builtins["ls"] = &com_ls;
    builtins["cd"] = &com_cd;
    builtins["pwd"] = &com_pwd;
    builtins["alias"] = &com_alias;
    builtins["unalias"] = &com_unalias;
    builtins["echo"] = &com_echo;
    builtins["exit"] = &com_exit;
    builtins["history"] = &com_history;
    
    // Specify the characters that readline uses to delimit words
    rl_basic_word_break_characters = (char *) WORD_DELIMITERS;
    
    // Tell the completer that we want to try completion first
    rl_attempted_completion_function = word_completion;
	
	// Print out greeting
	cout << "Initializing hsh, v1.1.0:\n  User: " << user() << "\n  Home: " << getenv("HOME") << "\n  PWD: " << pwd() << "\n\n";
	
	// Read in the history file
	int return_value = read_history(NULL);
	if (return_value != NORMAL_EXIT) {
		perror("Could not load history from disk!");
		cout << "Creating new blank history file." << endl;
		return_value = write_history(NULL);
		if (return_value != NORMAL_EXIT) {
			perror("Could not make new history file! History will not be persistent!");
		}
	}
	
	// Initialization complete message
	cout << "\nHsh initialization complete!\n\n";
}

int check_for_redirects(vector<string> tokens){
	d_cout("Checking for redirects in line\n");
	for (size_t i = 0; i < tokens.size(); i++) {
		d_cout("Found a redirect operator: ", tokens[i].c_str(), "\n");
		if (tokens[i] == "<") {
			if (std_in_redirect) {
				cerr << "Only one redirection is allowed per command\n";
				return MULTIPLE_REDIRECTIONS;
			}
			std_in_redirect = true;
		}
		else if (tokens[i] == ">") {
			if (std_out_redirect) {
				cerr << "Only one redirection is allowed per command\n";
				return MULTIPLE_REDIRECTIONS;
			}
			std_out_redirect = true;
		}
		else if (tokens[i] == ">>") {
			if (std_out_append_redirect) {
				cerr << "Only one redirection is allowed per command\n";
				return MULTIPLE_REDIRECTIONS;
			}
			std_out_append_redirect = true;
		}
	}
	// If any two flags are set, then return an error
	if ((std_in_redirect && (std_out_redirect || std_out_append_redirect))
		|| (std_out_redirect && (std_in_redirect || std_out_append_redirect))
		|| (std_out_append_redirect && (std_in_redirect || std_out_redirect))) {
		cerr << "Only one redirection is allowed per command\n";
		return MULTIPLE_REDIRECTIONS;
	}
	d_cout("Redirect check completed without errors\n");
	return NORMAL_EXIT;
}

int process_redirect(vector<string>tokens, int redirect_destination) {
	return ABNORMAL_EXEC;
}

// The main program
int main() {
	
	initializeShell();

    
    // The return value of the last command executed
    int return_value = 0;
	
	// The return value of the second last command
	int return_second_value = NOT_READY;
    
    // Loop for multiple successive commands
	d_cout("Command loop start\n");
    while (true) {
        
		// Reset the redirection flags
		std_in_redirect = false;
		std_out_redirect = false;
		std_out_append_redirect = false;
		
        // Get the prompt to show, based on the return value of the last command
        string prompt = get_prompt(return_value);
        
        // Read a line of input from the user
        char* line = readline(prompt.c_str());
        d_cout("Line read\n");
        // If the pointer is null, then an EOF has been received (ctrl-d)
        if (!line) {
            break;
        }
		d_cout("Passed null line check\n");
        // If the command is non-empty, attempt to execute it
        if (line[0]) {
			d_cout("Command not empty\n");
            // Break the raw input line into tokens
            vector<string> tokens = tokenize(line);
			d_cout("Input tokenized\n");

			d_cout("Substituting each token\n");
			
			int ret_val;
			
			for (int i = 0; i < tokens.size(); i++) {
				string return_string = history_substitution(tokens[i]);
				tokens[i] = return_string.c_str();
				d_cout("Substitution complete on token: ", tokens[i].c_str(), " got: ", return_string.c_str(), "\n");
			}
			
			d_cout("Substitution complete\n");
			
			d_cout("Adding to history\n");
			// Add this command to readline's history
			stringstream history_string;
			for (int i = 0; i < tokens.size(); i++) {
				history_string << tokens[i];
			}
			add_history(history_string.str().c_str());
			d_cout("Updating the history file\n");
			// Update history file
			if (write_history(NULL) != NORMAL_EXIT) {
				perror("Could not save history file!");
			}
			d_cout("Wrote history, passed return value check\n");

        
            // Handle local variable declarations
            local_variable_assignment(tokens);
            d_cout("Local variables assigned\n");
            // Substitute variable references
            variable_substitution(tokens);
            d_cout("Variables substituted\n");
			return_second_value = return_value;
			d_cout("Updated second return value\n");
			d_cout("----------------------EXECUTION OUTPUT----------------------\n");
            // Execute the line
            return_value = execute_line(tokens, builtins);
			d_cout("-------------------END EXECUTION OUTPUT---------------------\n");
			d_cout("Line completed execution\n");
			// If the exit shell signal is the return code, then close the shell
			if (return_value != NORMAL_EXIT) {
				switch (return_value) {
					case SIGNAL_EXIT_SHELL:
						free(line);
						return return_second_value;
						
					case CMD_NOT_FOUND:
						cerr << line << ": command not found\n";
						break;
						
					default:
						break;
				}
			}
			d_cout("Passed the return value checks\n");
        }
        // Free the memory for the input string
        free(line);
    }
    
    return NORMAL_EXIT;
}
