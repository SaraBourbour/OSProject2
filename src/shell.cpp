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

//   #include <signal.h>
//   #include <sys/errno.h>
//   #include <sys/param.h>

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
vector<string> redirect_operators;

// File redirect tokens, i.e. everthing after the command when a <, >, >> is found.
// Ex: `echo blah > file' would have {'>', "file"} in the vector
vector<string> redirect_tokens;

// char array to mark which redirect is active, can be used for multiple redirects
char redirect_flags[3];

// Handles external commands, redirects, and pipes.
int execute_external_command(vector<string> tokens) {
	
	int child_PID = -1;
	int *child_return_code = new int;
	// Fork off a new process
	d_printf("Forking a child process\n");
	child_PID = fork();
	// If this process is the child
	if (child_PID == 0) {
		d_printf("Inside the child process\n");
		// Generate the command char *const command[], with NULL on the end
		char *command[tokens.size() + 1];
		d_printf("Generating command array\n");
		for (int i = 0; i < tokens.size(); i++) {
			command[i] = (char*)tokens[i].c_str();
		}
		// Don't forget the null
		command[tokens.size()] = NULL;
		// Try execution with each path in $PATH
		d_printf("Preparing to path match the command\n");
		string path = getenv("PATH");
		string path_delimiter = ":";
		size_t current_position = 0;
		string path_component;
		bool executed = false;
		string file_to_execute;
		
		d_printf("Command[0]: %s\n", command[0]);
		while ((current_position = path.find(path_delimiter)) != string::npos && !executed) {
			path_component = path.substr(0, current_position);
			d_printf("Path component: %s\n", path_component.c_str());
			// Append the path component to the command
			file_to_execute = path_component + "/" + command[0];
			d_printf("Trying command: %s\n", file_to_execute.c_str());
			// Attempt to execute it
			int ret_val = execve(file_to_execute.c_str(), command, environ);
			d_printf("Exec returned: %d\n", ret_val);
			// If the execution was good
			if (ret_val != EXEC_FAIL) {
				// Mark executed so we stop
				executed = true;
				// Return normal exit code
				d_printf("Exiting with 0\n");
				exit(NORMAL_EXIT);
			}
			// Chop that part of the path string off
			path.erase(0, current_position + path_delimiter.length());
		}
		// Try execution with PWD for the command if PATH didn't find it
		if (!executed) {
			d_printf("Trying to execute with pwd\n");
			file_to_execute = pwd() + "/" + command[0];
			int ret_val = execve(file_to_execute.c_str(), command, environ);
			if (ret_val != EXEC_FAIL) {
				d_printf("PWD execution successful\n");
				executed = true;
				errno = 0;
				exit(NORMAL_EXIT);
			}
		}
		d_printf("Unable to execute, command not found\n");
		// If we still haven't executed, return command not found
		exit(CMD_NOT_FOUND);
	}
	// Else this is the parent
	else {
		d_printf("In parent process\n");
		// Wait for the child to exit
		wait(child_return_code);
		// Sleep for 100ms to alleviate race condition on stdin
		usleep(100000);
		d_printf("Child exited with code: %d. Resuming parent control\n", *child_return_code);
		if (*child_return_code != NORMAL_EXIT) {
			d_printf("Child exited abnormally\n");
			return *child_return_code;
		}
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

int exists_in_vector(vector<string> container, string token) {
	if (container.size() < 1) return -1;
	for (int i = 0; i < container.size(); i++) {
		if (container[i] == token) return i;
	}
	return -1;
}


// Transform a C-style string into a C++ vector of string tokens, delimited by
int tokenize(const char* line, vector<string>& save_to) {
    vector<string> tokens;
    string token;
    bool populate_redirect_tokens = false;
    // istringstream allows us to treat the string like a stream
    istringstream token_stream(line);
    d_printf("Tokenizing line\n");
    while (token_stream >> token) {
		int is_redirect = exists_in_vector(redirect_operators, token);
		if (is_redirect != -1) {
			if (populate_redirect_tokens) {
				cerr << "Multiple redirects are not allowed\n";
				tokens.clear();
				save_to = tokens;
				return MULTIPLE_REDIRECTS;
			}
			d_printf("Found a redirect operator: %s\n", redirect_operators[is_redirect].c_str());
			populate_redirect_tokens = true;
		}
		if (populate_redirect_tokens) {
			d_printf("Pushing back to redirect tokens: %s\n", token.c_str());
			redirect_tokens.push_back(token);
		}
		else {
			d_printf("Pushing back to tokens: %s\n", token.c_str());
			tokens.push_back(token);
		}
    }
    
    // Search for quotation marks, which are explicitly disallowed
    for (size_t i = 0; i < tokens.size(); i++) {
        
        if (tokens[i].find_first_of("\"'`") != string::npos) {
            cerr << "\", ', and ` characters are not allowed." << endl;
            
            tokens.clear();
        }
    }
    
	// Ensure the redirect token length is only 0 or 2 long
	d_printf("Redirect tokens size: %d\n", (int)redirect_tokens.size());
	if ((int)redirect_tokens.size() != 0 && (int)redirect_tokens.size() != 2) {
		cerr << "Invalid redirect syntax\n";
		tokens.clear();
		save_to = tokens;
		return BAD_REDIRECT;
	}
	
	d_printf("Tokenizing complete\n");
	save_to = tokens;
    return NORMAL_EXIT;
}


// Executes a line of input by either calling execute_external_command or
// directly invoking the built-in command.
int execute_line(vector<string>& tokens, map<string, command>& builtins) {
    int return_value = 0;
    
    if (tokens.size() != 0) {
		int stdin_fd = dup(STD_IN);
		int stdout_fd = dup(STD_OUT);
		int in_fd, out_fd, append_fd;
		// Prepare file descriptors, if needed
		if (redirect_flags[REDIRECT_IN] == 1) {
			in_fd = open(redirect_tokens[1].c_str(), O_RDONLY, 0);
			dup2(in_fd, STD_IN);
			close(in_fd);
		}
		else if (redirect_flags[REDIRECT_OUT] == 1) {
			out_fd = open(redirect_tokens[1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2(out_fd, STD_OUT);
			close(out_fd);
		}
		else if (redirect_flags[REDIRECT_APPEND] == 1) {
			append_fd = open(redirect_tokens[1].c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
			dup2(append_fd, STD_OUT);
			close(append_fd);
		}
		
        map<string, command>::iterator cmd = builtins.find(tokens[0]);
        
        if (cmd == builtins.end()) {
			d_printf("Could not find an internal command, trying external\n");
            int ret_val = execute_external_command(tokens);
			fflush(stdin);
			fflush(stdout);
			close(in_fd);
			close(out_fd);
			close(append_fd);
			dup2(stdin_fd, STD_IN);
			dup2(stdout_fd, STD_OUT);
			close(stdin_fd);
			close(stdout_fd);
			return ret_val;
			
        } else {
			int ret_val = ((*cmd->second)(tokens));
			// Flush and reset file descriptors
			fflush(stdout);
			close(out_fd);
			dup2(stdout_fd, STD_OUT);
			close(stdout_fd);
			return ret_val;
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
	d_printf("Beginning history substitution for token: %s\n", token.c_str());
	char** new_tokens = new char*[1];
	int ret_val = history_expand((char*)token.c_str(), new_tokens);
	d_printf("Expansion complete. Return code: %d\n", ret_val);
	
	if (ret_val != NORMAL_EXIT_EXPANSION) {
		d_printf("Token: %s not substituted\n", token.c_str());
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
	printf("Initializing hsh, v1.1.0:\n  User: %s\n  Home: %s\n  PWD: %s\n\n", user().c_str(), getenv("HOME"), pwd().c_str());
	
	// Read in the history file
	int return_value = read_history(NULL);
	if (return_value != NORMAL_EXIT) {
		perror("Could not load history from disk!");
		printf("Creating new blank history file.\n");
		return_value = write_history(NULL);
		if (return_value != NORMAL_EXIT) {
			perror("Could not make new history file! History will not be persistent!");
		}
	}
	redirect_operators.push_back("<");
	redirect_operators.push_back(">");
	redirect_operators.push_back(">>");
	
	// Initialization complete message
	printf("\nHsh initialization complete!\n\n");
}

void set_redirect_flags() {
	d_printf("Setting redirect flags\n");
	redirect_flags[0] = redirect_flags[1] = redirect_flags[2] = 0;
	d_printf("Redirect flags reset\n");
	if (redirect_tokens.size() == 0) return;
	if (redirect_tokens[0] == redirect_operators[0]) {
		d_printf("Setting input redirect\n");
		redirect_flags[0] = 1;
	}
	else if (redirect_tokens[0] == redirect_operators[1]) {
		d_printf("Setting output redirect\n");
		redirect_flags[1] = 1;
	}
	else if (redirect_tokens[0] == redirect_operators[2]) {
		d_printf("Setting append redirect\n");
		redirect_flags[2] = 1;
	}
}

int handle_pipes(vector<string> tokens) {
	bool pipe = false;
	int remove_to_index = 0;
	// Look for pipes
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "|") {
			pipe = true;
			remove_to_index = i;
			break;
		}
	}
	// If a pipe is found, execute only the first part here
	if (pipe) {
		int child_PID = -1;
		// Fork the rest
		child_PID = fork();
		if (child_PID == 0) {
			// In child
		}
		else {
			// In parent
		}
		// Restart this function with everything up to pipe removed
		handle_pipes(
	}
	// If no pipe is found, execute normally (recursive stopping condition)
	else {
		
	}
	return NORMAL_EXIT;
}

// The main program
int main() {
	
	initializeShell();

    
    // The return value of the last command executed
    int return_value = 0;
	
	// The return value of the second last command
	int return_second_value = NOT_READY;
    
    // Loop for multiple successive commands
	d_printf("Command loop start\n");
    while (true) {
		
        // Get the prompt to show, based on the return value of the last command
        string prompt = get_prompt(return_value);
        
        // Read a line of input from the user
        char* line = readline(prompt.c_str());
        d_printf("Line read\n");
        // If the pointer is null, then an EOF has been received (ctrl-d)
        if (!line) {
            break;
        }
		d_printf("Passed null line check\n");
        // If the command is non-empty, attempt to execute it
        if (line[0]) {
			d_printf("Command not empty\n");
            // Break the raw input line into tokens
            vector<string> tokens;
			redirect_tokens.clear();
			int tokenize_return_value = tokenize(line, tokens);
			if (tokenize_return_value != NORMAL_EXIT) {
				return_second_value = return_value;
				return_value = tokenize_return_value;
				free(line);
				continue;
			}
			d_printf("Input tokenized\n");
			set_redirect_flags();
			d_printf("Substituting each token\n");
			
			int ret_val;
			
			for (int i = 0; i < tokens.size(); i++) {
				string return_string = history_substitution(tokens[i]);
				tokens[i] = return_string.c_str();
				d_printf("Substitution complete on token: %s. Got: %s\n", tokens[i].c_str(), return_string.c_str());
			}
			
			d_printf("Substitution complete\n");
			
			d_printf("Adding to history\n");
			// Add this command to readline's history
			stringstream history_string;
			for (int i = 0; i < tokens.size(); i++) {
				history_string << tokens[i];
			}
			add_history(history_string.str().c_str());
			d_printf("Updating the history file\n");
			// Update history file
			if (write_history(NULL) != NORMAL_EXIT) {
				perror("Could not save history file!");
			}
			d_printf("Wrote history, passed return value check\n");

        
            // Handle local variable declarations
            local_variable_assignment(tokens);
            d_printf("Local variables assigned\n");
            // Substitute variable references
            variable_substitution(tokens);
			
			handle_pipes(tokens);
			
			
            d_printf("Variables substituted\n");
			return_second_value = return_value;
			d_printf("Updated second return value\n");
			d_printf("----------------------EXECUTION OUTPUT----------------------\n");
            // Execute the line
            return_value = execute_line(tokens, builtins);
			d_printf("-------------------END EXECUTION OUTPUT---------------------\n");
			d_printf("Line completed execution\n");
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
			d_printf("Passed the return value checks\n");
        }
        // Free the memory for the input string
        free(line);
    }
    
    return NORMAL_EXIT;
}
