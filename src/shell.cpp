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



// Handles external commands, redirects, and pipes.
int execute_external_command(vector<string> tokens) {
    // TODO: YOUR CODE GOES HERE
    return CMD_NOT_FOUND;
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
    
    while (token_stream >> token) {
        tokens.push_back(token);
    }
    
    // Search for quotation marks, which are explicitly disallowed
    for (size_t i = 0; i < tokens.size(); i++) {
        
        if (tokens[i].find_first_of("\"'`") != string::npos) {
            cerr << "\", ', and ` characters are not allowed." << endl;
            
            tokens.clear();
        }
    }
    
    return tokens;
}


// Executes a line of input by either calling execute_external_command or
// directly invoking the built-in command.
int execute_line(vector<string>& tokens, map<string, command>& builtins) {
    int return_value = 0;
    
    if (tokens.size() != 0) {
        map<string, command>::iterator cmd = builtins.find(tokens[0]);
        
        if (cmd == builtins.end()) {
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

char* history_substitution(char* char_line) {
	debug_cout("Beginning history substitution\n");
	
	if (history_length == 0) {
		debug_cout("History has 0 length, stop substitution\n");
		return char_line;
	}
	
	debug_cout("Passed 0 length history check\n");
	string string_line = char_line;
	int offset = 0;
	bool changed = false;
	size_t found = 0;
	HIST_ENTRY* last_command_entity = history_get(history_length);
	const char* last_command = NULL;
	debug_cout("Fields initialized\n");
	
	if (last_command_entity != NULL) {
		last_command = last_command_entity->line;
		debug_cout("Entry not null, line initialized\n");
	}
	else {
		debug_cout("Entry was null, line NOT initialized\n");
		return NULL;
	}
	
	// Loop to find all the !! commands and replace them
	debug_cout("Starting first pass\n");
	while (found != string::npos) {
		found = string_line.find("!!");
		if (found == string::npos) {
			// No more !!'s, stop loop
			debug_cout("No !! found, stopping first pass\n");
			break;
		}
		if (found != string::npos) {
			debug_cout("Found a !!, replacing it with last command\n");
			string_line.replace(found, 2, last_command);
			changed = true;
		}
	}
	
	debug_cout("First pass complete\n");
	found = 0;
	debug_cout("Starting second pass\n");
	while (found != string::npos) {
		found = string_line.find("!");
		if (found == string::npos) {
			// No more !'s, stop loop
			debug_cout("No !# found, stopping second pass\n");
			break;
		}
		debug_cout("Initializing pass fields\n");
		string string_offset = "";
		int offset;
		bool negate = false;
		// Generate offset, as a string
		debug_cout("Generating offset\n");
		for (int i = found + 1; i < string_line.length(); i++) {
			if (string_line[i] == '-') {
				debug_cout("Found '-', negating the offset\n");
				negate = true;
				continue;
			}
			if (!isdigit(string_line[i])) {
				debug_cout("Found end of number\n");
				break;
			}
			string_offset += string_line[i];
			debug_cout("Updated string offset: " + string_offset + "\n");
		}
		// If no offset was generated
		if (string_offset == "") {
			debug_cout("No offset found, leave the ! alone, stop substitution");
			break;
		}
		
		offset = atoi(string_offset.c_str());
		stringstream debug;
		debug << "Offset generated: " << offset << "\n";
		debug_cout(debug.str());
		
		if (negate) {
			if (offset > history_length) {
				cerr << "!-" << offset << ": event not found" << endl;
				debug_cout("Second pass complete\n");
				return NULL;
			}
			else {
				HIST_ENTRY *temp_entry = history_get(history_length - offset);
				if (temp_entry == NULL) {
					cerr << "!-" << offset << ": event not found" << endl;
					debug_cout("Second pass complete\n");
					return NULL;
				}
				const char* history_command = temp_entry->line;
				string_line.replace(found, sizeof(history_command), history_command);
				debug_cout("Substituted line element\n");
			}
		}
		else {
			if (offset > history_length) {
				cerr << "!" << offset << ": event not found" << endl;
				debug_cout("Second pass complete\n");
				return NULL;
			}
			else {
				HIST_ENTRY *temp_entry = history_get(offset);
				if (temp_entry == NULL) {
					cerr << "!" << offset << ": event not found" << endl;
					debug_cout("Second pass complete\n");
					return NULL;
				}
				const char* history_command = temp_entry->line;
				string_line.replace(found, sizeof(history_command), history_command);
				debug_cout("Substituted line element\n");
			}
		}
	}
	debug_cout("Second pass complete\n");
	// Don't leak the previous line
	free(char_line);
	// Create a new space for the line
	char_line = new char[string_line.size()];
	// Copy over contents of newLine
	memcpy(char_line, string_line.c_str(), string_line.size());
	// If the line was changed print it out, like BASH
	stringstream debug;
	debug << "New char_line generated: " << char_line << "\n";
	debug_cout(debug.str());
	
	if (changed) {
		cout << char_line << endl;
	}
	// Return the new line
	return char_line;
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
	cout << "Initializing hsh, v1.0.0:\n  User: " << user() << "\n  Home: " << getenv("HOME") << "\n  PWD: " << pwd() << "\n\n";
	
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


// The main program
int main() {
	
	initializeShell();

    
    // The return value of the last command executed
    int return_value = 0;
	
	// The return value of the second last command
	int return_second_value = NOT_READY;
    
    // Loop for multiple successive commands
	debug_cout("Command loop start\n");
    while (true) {
        
        // Get the prompt to show, based on the return value of the last command
        string prompt = get_prompt(return_value);
        
        // Read a line of input from the user
        char* line = readline(prompt.c_str());
        debug_cout("Line read\n");
        // If the pointer is null, then an EOF has been received (ctrl-d)
        if (!line) {
            break;
        }
		debug_cout("Passed null line check\n");
        // If the command is non-empty, attempt to execute it
        if (line[0]) {
			debug_cout("Command not empty\n");
			// Handle history substitutions
			line = history_substitution(line);
			stringstream debug;
			debug <<"Subs completed, line: " << line << endl;
			debug_cout(debug.str());
			// If null, there was a substitution error
			if (!line) {
				debug_cout("SUB ERROR!!\n");
				return_second_value = return_value;
				return_value = BAD_SUBSTITUTION;
				continue;
			}
			debug_cout("Passed the null line check\n");
            // Break the raw input line into tokens
            vector<string> tokens = tokenize(line);
			debug_cout("Input tokenized\n");
			if (tokens[0] != "history") {
				debug_cout("Adding to history\n");
				// Add this command to readline's history
				add_history(line);
				debug_cout("Added to history\n");
				// Update history file
				if (write_history(NULL) != NORMAL_EXIT) {
					perror("Could not save history file!");
				}
				debug_cout("Wrote history, passed return value check\n");
			}
        
            // Handle local variable declarations
            local_variable_assignment(tokens);
            debug_cout("Local variables assigned\n");
            // Substitute variable references
            variable_substitution(tokens);
            debug_cout("Variables substituted\n");
			return_second_value = return_value;
			debug_cout("Updated second return value\n");
			debug_cout("----------------------EXECUTION OUTPUT----------------------\n");
            // Execute the line
            return_value = execute_line(tokens, builtins);
			debug_cout("-------------------END EXECUTION OUTPUT---------------------\n");
			debug_cout("Line completed execution\n");
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
			debug_cout("Passed the return value checks\n");
        }
        // Free the memory for the input string
        free(line);
    }
    
    return NORMAL_EXIT;
}
