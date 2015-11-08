//
//  Purpose: A command line interpreter (shell) written in C, for CSC2025 Assignment 1
//           (based off of slide 7 of the CSC2025 part 2 Lecture Notes)
//  Author: Dylan McKee
//  Date: 08/11/2015
//

#include <stdio.h>
#include <stdbool.h>

// Declare function prototypes
void prompt();
void readcmd(char *cmd, char *params[]);

int main(int argc, const char * argv[]) {
    
    // Repeat forever
    while (true) {
        // Prompt the user for some input...
        prompt();
        
        // Read the user's input...
        readcmd(NULL, NULL);
        
        int status = 0;
        
        // Fork a process, or wait for the currently excecuting one to complete...
        if (fork() != 0) {
            // This is the parent process, wait for the child process to complete...
            waitpid(-1, &status, 0);
        } else {
            // This is the child process, execute the user's command with any parameters they've passed in...
            
        }
        
    }
    
    return 0;
}


// A function to print the shell's command prompt to the console...
void prompt() {
    // Print out some instructions...
    printf("Enter your command & parameters, then press enter:");
    
}

// A function to read an input from the console, split it into command and parameters, and read these values into the pointers passed into it...
void readcmd(char *cmd, char *params[]) {
    // Get the user input...
    // I looked up the use of the getline() function at http://c-for-dummies.com/blog/?p=1112
    
    // Give the buffer a reasonable size...
    size_t bufferSize = 1024;
    
    // And allocate the buffer dynamically...
    char *buffer = (char*) malloc(bufferSize * sizeof(char));
    
    // Passing stdin as getline()'s FILE reference
    // Read a line from stdin into the buffer we've allocated...
    size_t line = getline(&buffer, &bufferSize, stdin);
    
    // -1 return from getline indicates an error, check return code...
    if (line == -1) {
        // error!
        printf("An error occured. Please restart the shell and try again");
        
        // and exit abruptly...
        exit(1);
    }
    
    // Split the line by string...
    // (I looked up the strtok function at http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm )
    char *inputCommand = NULL;
    
    const char seperator[2] = " ";
    char *inputToken = strtok(inputCommand, seperator);
    
    
    
    while (inputToken != NULL) {
        if (inputCommand == NULL) {
            strcpy(inputCommand, inputToken);
        }
    }
    
    
    
    
    
}