#include "builtins.h"


using namespace std;


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
	if (LOW_LEVEL_DEBUG) {
		debug_cout("All tokens in cd: ");
		for (int i=0; i<tokens.size(); i++) {
			debug_cout(tokens[i] + " ");
		}
		debug_cout("\n");
	}
    if (tokens.size() == 2) {
        // If the first value is a '/', using an absolute path
        if (tokens[1][0] == '/') {
			debug_cout("Changing to: " + tokens[1] + "\n");
            int ret_val = chdir(tokens[1].c_str());
			stringstream debug;
			debug << "Return value of chdir: " << ret_val << "\n";
			debug_cout(debug.str());
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
	debug_cout("In history\n");
	if (history_length == 0) {
		return NORMAL_EXIT;
	}
	debug_cout("Passed no history check\n");
	HIST_ENTRY *tempHistoryEntry = NULL;
	debug_cout("Temp entry created\n");
    if (tokens.size() > 2) {
		perror("history");
		return TOO_MANY_ARGUMENTS;
	}
	else if (tokens.size() == 2) {
		debug_cout("Passed too many arguments check\n");
		debug_cout("Two tokens found\n");
		// Show amount of history
		if (atoi(tokens[1].c_str()) > history_length) {
			perror("history");
			return INVALID_ARGUMENTS;
		}
		debug_cout("Passed too large number check\n");
		int show_amount = atoi(tokens[1].c_str());
		debug_cout("Show amount calculated\n");
		for (int i = 0; i < history_length - show_amount; i++) {
			tempHistoryEntry = history_get(i);
			stringstream debug;
			debug << "Got a new history element at: " << i << "\n";
			debug_cout(debug.str());
			if (tempHistoryEntry == NULL) {
				debug_cout("Element was null!\n");
				// Silenced error, this is okay. It's for compatibility for Linux vs BSD
//				perror("Trying to parse a null history pointer, moving on:");
				continue;
			}
			cout << "   " << i << "  " << history_get(i)->line << endl;
		}
	}
	else if (tokens.size() == 1) {
		debug_cout("Passed too many arguments check\n");
		debug_cout("Found only one token\n");
		for (int i = 0; i < history_length; i++) {
			tempHistoryEntry = history_get(i);
			stringstream debug;
			debug << "Got a new history element at: " << i << "\n";
			debug_cout(debug.str());
			if (tempHistoryEntry == NULL) {
				debug_cout("Element was null!\n");
				// Silenced error, this is okay. It's for compatibility for Linux vs BSD
//				perror("Trying to parse a null history pointer, moving on:");
				continue;
			}
			cout << "   " << i << "  " << history_get(i)->line << endl;
		}
	}
	else {
		debug_cout("WTF?! NEGATIVE ARRAY SIZE!?\n");
		// This should never happen, implies negative size
		perror("history");
		return ABNORMAL_EXEC;
	}
    return NORMAL_EXIT;
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

void debug_cout(string output) {
	if (LOW_LEVEL_DEBUG) {
		cout << output;
	}
}