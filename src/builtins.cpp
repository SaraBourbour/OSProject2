#include "builtins.h"

#define NORMAL_EXIT         int(0)
#define BAD_FILE_OR_DIR     int(1)
#define INVALID_ARGUMENTS   int(2)

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
    if (tokens.size() != 1) {
        perror("pwd");
        return INVALID_ARGUMENTS;
    }
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
    // TODO: YOUR CODE GOES HERE
    cout << "echo called" << endl; // delete when implemented
    return NORMAL_EXIT;
}


int com_exit(vector<string>& tokens) {
    // TODO: YOUR CODE GOES HERE
    cout << "exit called" << endl; // delete when implemented
    return NORMAL_EXIT;
}


int com_history(vector<string>& tokens) {
    // TODO: YOUR CODE GOES HERE
    cout << "history called" << endl; // delete when implemented
    return NORMAL_EXIT;
}

string pwd() {
    return getcwd(NULL, 0);
}
