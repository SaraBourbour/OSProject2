#include "builtins.h"


using namespace std;

stack<string> *directory_stack = new stack<string>();

int com_ls(vector<string>& tokens) {
    
    // if no directory is given, use the local directory
    if (tokens.size() < 2) {
        tokens.push_back(".");
    }
    
    // open the directory
    DIR* dir = opendir(tokens[1].c_str());
    
    // catch an errors opening the directory
    if (!dir) {
        // print the error from the last system call with the given prefix
        perror("ls");
        
        // return error
        return BAD_FILE_OR_DIR;
    }
    
    // output each entry in the directory
    for (dirent* current = readdir(dir); current; current = readdir(dir)) {
        cout << current->d_name << endl;
    }
    
    // return success
    return NORMAL_EXIT;
}


int com_cd(vector<string>& tokens) {
	if (DEBUGGING_ENABLED) {
		d_cout("All tokens in cd: ");
		for (int i=0; i<tokens.size(); i++) {
			d_cout("", tokens[i].c_str(), " ");
		}
		d_cout("\n");
	}
    if (tokens.size() == 2) {
        // If the first value is a '/', using an absolute path
        if (tokens[1][0] == '/') {
			d_cout("Changing to: ", tokens[1].c_str(), "\n");
			d_cout("Pushing pwd onto the directory stack");
			directory_stack->push(pwd());
            int ret_val = chdir(tokens[1].c_str());
			d_cout("Return value of chdir: ", ret_val, "\n");
			if (ret_val != NORMAL_EXIT) {
				perror("cd");
			}
            return ret_val;
        }
		else if (tokens[1][0] == '-') {
			d_cout("Changing to previous directory\n");
			int ret_val = chdir(directory_stack->top().c_str());
			directory_stack->pop();
			if (ret_val != NORMAL_EXIT) {
				perror("cd");
			}
			return ret_val;
		}
        else {
            string cwd = getcwd(NULL, 0);
            // Add a slash
            cwd += "/";
            // Add the new directory(ies)
            cwd += tokens[1];
            // Check the new_cwd exists before going there
            if (!opendir(cwd.c_str())) {
                perror("cd");
                return BAD_FILE_OR_DIR;
            }
            else {
                chdir(cwd.c_str());
                return NORMAL_EXIT;
            }
        }
    }
	else if (tokens.size() == 1) {
		d_cout("Pushing pwd onto the directory stack\n");
		directory_stack->push(pwd());
		chdir(getenv("HOME"));
		return NORMAL_EXIT;
	}
    // Too many arguemnts
    else {
        perror("cd");
        return INVALID_ARGUMENTS;
    }
}


int com_pwd(vector<string>& tokens) {
    cout << pwd() << endl;
    return NORMAL_EXIT;
}


int com_alias(vector<string>& tokens) {
    // TODO: YOUR CODE GOES HERE
    cout << "alias called" << endl; // delete when implemented
    return 1;
}


int com_unalias(vector<string>& tokens) {
    // TODO: YOUR CODE GOES HERE
    cout << "unalias called" << endl; // delete when implemented
    return NORMAL_EXIT;
}


int com_echo(vector<string>& tokens) {
	// If there's nothing to echo, echo nothing
	if (tokens.size() == 1) {
		cout << endl;
		return NORMAL_EXIT;
	}
	// Else we hav to echo something
	else {
		// Parse all the tokens in the echo statement
		for (int i = 1; i < tokens.size(); ++i) {
			string tempToken = tokens[i];
			if (i == tokens.size() - 1) {
				cout << tokens[i] << endl;
				return NORMAL_EXIT;
			}
			else {
				cout << tokens[i] << " ";
				//TODO: Stop if a pipe is encountered
			}
		}
	}
	// If echo does not run correctly, return abnormally
	return ABNORMAL_EXEC;
}


int com_exit(vector<string>& tokens) {
    return SIGNAL_EXIT_SHELL;
}


int com_history(vector<string>& tokens) {
	d_cout("In history\n");
	if (history_length == 0) {
		return NORMAL_EXIT;
	}
	d_cout("Passed no history check\n");
    if (tokens.size() > 2) {
		perror("history");
		return TOO_MANY_ARGUMENTS;
	}
	else if (tokens.size() == 2) {
		d_cout("Passed too many arguments check\n", "Two tokens found\n", "Ensuring argument is not negative\n");
		if (tokens[1][0] == '-') {
			cerr << "history: cannot have a negative argument\n";
			return INVALID_ARGUMENTS;
		}

		// Show amount of history
		print_last_amount_history(atoi(tokens[1].c_str()));
	}
	else if (tokens.size() == 1) {
		d_cout("Passed too many arguments check\n", "Found only one token\n");
		print_last_amount_history(history_length);
	}
	else {
		d_cout("WTF?! NEGATIVE ARRAY SIZE!?\n");
		// This should never happen, implies negative size
		perror("history");
		return ABNORMAL_EXEC;
	}
    return NORMAL_EXIT;
}

// Precondition: Expects a positive offset, for amount of elements to display
void print_last_amount_history(int amount) {
	// Cap amount of history to display
	if (amount > history_length) {
		amount = history_length;
	}
	HIST_ENTRY *tempHistoryEntry = NULL;
	d_cout("Temp entry created\n");
	for (int i = history_length - amount; i <= history_length; i++) {
		tempHistoryEntry = history_get(i);
		d_cout("\nGot a new history element at: ", i, "\n");
		if (tempHistoryEntry == NULL) {
			d_cout("Element was null!\n");
			// Silenced error, this is okay. It's for compatibility for Linux vs BSD
			//				perror("Trying to parse a null history pointer, moving on:");
			continue;
		}
		else {
			cout << "   " << i << "  " << tempHistoryEntry->line << endl;
		}
	}
}

string pwd() {
    return getcwd(NULL, 0);
}

string user() {
	return getenv("USER");
}
string last_command_status(int code) {
	stringstream ss;
	ss << "lc: ";
	ss << code;
	return ss.str();
}

void debug_cout(const char *arg, ... ) {
	if (DEBUGGING_ENABLED) {
		va_list arguments;
		for (va_start(arguments, arg); arg != NULL; arg = va_arg(arguments, const char *)) {
			cout << arg;
		}
		va_end(arguments);
	}
}