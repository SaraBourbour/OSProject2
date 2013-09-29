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
    
    if (tokens.size() == 2) {
        // If the first value is a '/', using an absolute path
        if (tokens[1][0] == '/') {
			cout << "Changing to: " << tokens[1] << endl;
            chdir(tokens[1].c_str());
            return NORMAL_EXIT;
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
    // Too many arguemnts
    else {
        perror("cd");
        return INVALID_ARGUMENTS;
    }
}


int com_pwd(vector<string>& tokens) {
    // HINT: you should implement the actual fetching of the current directory in
    // pwd(), since this information is also used for your prompt
    // There must not be any parameters to pwd
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
	// Save the history
	int return_value = write_history(NULL);
	if (return_value != NORMAL_EXIT) {
		perror("Could not save history file!");
	}
    return SIGNAL_EXIT_SHELL;
}


int com_history(vector<string>& tokens) {
	cout << "In history" << endl;
	HIST_ENTRY *tempHistoryEntry = NULL;
    if (tokens.size() > 2) {
		perror("history");
		return TOO_MANY_ARGUMENTS;
	}
	else if (tokens.size() == 2) {
		cout << "Found 2 tokens" << endl;
		// Show amount of history
		if (atoi(tokens[1].c_str()) > history_length) {
			perror("history");
			return INVALID_ARGUMENTS;
		}
		int show_amount = atoi(tokens[1].c_str());
		cout << "Show amount calculated" << endl;
		for (int i = 0; i < history_length - show_amount; i++) {
			tempHistoryEntry = history_get(i);
			if (tempHistoryEntry == NULL) {
				perror("Trying to parse a null history pointer, moving on.");
				continue;
			}
			cout << "   " << i << "  " << history_get(i)->line << endl;
		}
	}
	else if (tokens.size() == 1) {
		cout << "Found only one token" << endl;
		cout << "Hitsory Length: " << history_length << endl;
		for (int i = 0; i < history_length; i++) {
			cout << "On: " << i << endl;
			tempHistoryEntry = history_get(i);
			if (tempHistoryEntry == NULL) {
				perror("Trying to parse a null history pointer, moving on.");
				continue;
			}
			cout << "   " << i << "  " << history_get(i)->line << endl;
		}
	}
	else {
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