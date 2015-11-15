/*
 * Dylan McKee - 140357185
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Define some constants
// The maximum command input length in chars (for buffer size)...
#define INPUT_BUFFER_SIZE 1024

// Declare function prototypes
void prompt();
void readcmd(char *cmd, char *params[]);

// Declare the string that quits the shell as a constant
const char *quitString = "q";

int main(int argc, const char * argv[]) {
    
    // Repeat forever
    while (true) {
        // Prompt the user for some input...
        prompt();
        
        // Create a command string placeholder pointer... (only needs to exist within the scope of this while loop)
        char *commandString = malloc(INPUT_BUFFER_SIZE);
        
        // Read the user's input...
        readcmd(commandString, NULL);
        
        // Check if the user entered anything at the prompt or just hit return?
        // See if the first char in the string is a newline - if so - they just hit return.
        if (strcmp(commandString, "\n") == 0) {
            // The user just hit return; loop around, don't try to execve a newline char...
            continue;
        }
        
        // Now that we're certain they entered a command, remove the trailing \n from the command string (if one exists)...
        commandString = strtok(commandString, "\n");
        
        printf(commandString);

        // TODO: Remove possible trailing \n from the last argument too!
        
        // If the user enters 'q' at the prompt, exit the shell normally
        if (strcmp(commandString, quitString) == 0) {
            // The user entered 'q' - quit normally!
            printf("\nYou entered 'q' - exiting the shell.\n");
            exit(0);
        }
        
        // Initialise a placeholder variable to read the process status number into
        int status = 0;
        
        // Fork a process, or wait for the currently excecuting one to complete...
        if (fork() != 0) {
            // This is the parent process, wait for the child process to complete, read status into status placeholder var...
            waitpid(-1, &status, 0);
        } else {
            // This is the child process, execute the user's command with any parameters they've passed in...
            execve(commandString, NULL, NULL);
        }
        
        // Free the command string pointer; we've finished with it and need to clear it from the heap
        free(commandString);
        
    }
    
    return 0;
}


// A function to print the shell's command prompt to the console...
void prompt() {
    // Get the current working directory
    // I looked up the getcwd() function at http://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html
    size_t cwdBufferSize = INPUT_BUFFER_SIZE;

    // Allocate a buffer of a reasnoble size to read the current working directory path string into
    char *cwdBuffer = malloc(cwdBufferSize * sizeof(char));
    
    // Get the current working directory path into
    getcwd(cwdBuffer, cwdBufferSize);
    
    // Print the current working directory's path to the console
    printf(cwdBuffer);
    
    // And print a colon then a space so the user can easily see where they're entering their command...
    printf(": ");
    
}


// A function to read an input from the console, split it into command and parameters, and read these values into the pointers passed into it...
void readcmd(char *cmd, char *params[]) {
    // Get the user input...
    // I looked up the use of the getline() function at http://c-for-dummies.com/blog/?p=1112
    
    // Give the buffer a reasonable size...
    size_t bufferSize = INPUT_BUFFER_SIZE;
    
    // And allocate the buffer dynamically, multiplying the desired buffer size by the size of a char then allocating that much memory on the heap...
    char *buffer = malloc(bufferSize * sizeof(char));
    
    // Passing stdin as getline()'s FILE reference
    // Read a line from stdin into the buffer we've allocated...
    size_t line = getline(&buffer, &bufferSize, stdin);
    
    // a -1 return code from getline indicates an error, check if an error occured...
    if (line == -1) {
        // error!
        printf("An error occured. Please restart the shell and try again");
        
        // and exit abruptly...
        exit(1);
    }
    
    // Split the line by string...
    // (I looked up the strtok function at http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm )
    char *inputCommand = malloc(bufferSize * sizeof(char));
    
    // We want to separate the string based on spaces...
    const char *seperator = " ";
    
    // Start the tokenizer by passing in the buffer string and the space seperator...
    char *inputToken = strtok(buffer, seperator);
    
    // The input command will be the first token but has not yet been set - this boolean acts as a flag, so we know when it has been set and do not overwrite it
    bool inputCommandSet = false;
    
    // While there's still tokens to be read in, read them!
    while (inputToken != NULL) {
        if (inputCommandSet == false) {
            // Copy the token into the input command
            strcpy(inputCommand, inputToken);
            
            // and set the flag
            inputCommandSet = true;
            
        } else {
            // it's a parameter, add it to the parameters array and resize
            printf("param:%s", inputToken);
        }
        
        // And read the next token...
        inputToken = strtok(NULL, seperator);
        
    }
    
    
    // Copy the value of the input command string we read into the cmd pointer we've been passed (do not assign directly because inputCommand's a local pointer)
    strcpy(cmd, inputCommand);
    
    // Free local dynamically allocated pointers from the heap!
    free(buffer);
    free(inputToken);
    free(inputCommand);
    
}


