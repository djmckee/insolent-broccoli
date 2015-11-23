/*
 * Dylan McKee - 140357185
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// Define some constants
// The maximum command input length in chars (for buffer size)...
#define INPUT_BUFFER_SIZE 1024

// The maximum parameter input length in chars
#define PARAMETER_INPUT_LENGTH_MAX 255

// Define the string that quits the shell as a constant
const char *QUIT_STRING = "q";

// Define the newline char. as a constant to improve code readability and make it easy to change for example if this code is used on a non-standard system with a different newline char to \n
const char *NEW_LINE_CHAR = "\n";

// Define a command to use for help...
const char *HELP_COMMAND = "help";

// Define a constant 'help' string to print to the console when the user uses the help command
const char *HELP_STRING = "\nHELP\nThis is a simple shell. To use, enter your command and any parameters (separated by spaces) and hit return.\nEnter 'q' and hit return to exit the shell.\n\n";

// Declare function prototypes
void prompt();
void readCmd(char *cmd, char **params[]);
void detectQuitCmd(char *cmd);
bool executeBuiltIn(char *cmd, char *params[]);
void printCurrentDirToConsole();

// Declare built-in function prototypes
void builtinCd(char *params[]);
void builtinPwd();

// I based the rough structure of the main() function, along with the initial function prototype names, off of the shell slide in the lecture notes.
int main(int argc, const char *argv[] ) {
    
    // Repeat forever
    while (true) {
        // Prompt the user for some input...
        prompt();
        
        // Create a command string placeholder pointer... (only needs to exist within the scope of this while loop)
        char *commandString = malloc(INPUT_BUFFER_SIZE);
        
        // Create a parameters array placeholder pointer - this will be dynamically resized via realloc in the readcmd() function, but just needs to be initialised to the size of a NULL (so the NULL terminator can fit in, if there's no parameters, without any resizing going on)...
        char **parametersArray = malloc(sizeof(NULL));
        
        // Read the user's input...
        readCmd(commandString, parametersArray);
        
        // Check if the user entered anything at the prompt or just hit return?
        // See if the first char in the string is a newline - if so - they just hit return.
        if (strcmp(commandString, NEW_LINE_CHAR) == 0) {
            // The user just hit return; loop around, don't try to execve a newline char...
            continue;
        }
        
        // Now that we're certain they entered a command, remove the trailing \n from the command string (if one exists)...
        commandString = strtok(commandString, NEW_LINE_CHAR);
        
        // If the user asked for help, print the help text to the console, then continue on the while loop...
        if (strcmp(commandString, HELP_COMMAND) == 0) {
            // Print some pre-defined help
            printf("%s", HELP_STRING);
            
            // Loop around...
            continue;
        }
        
        // If the command is built-in, execute it using the built-in function and continue on in the while loop...
        if (executeBuiltIn(commandString, parametersArray)) {
            continue;
        }
        
        
        // Newline so we print output out on another line
        printf("\n");
        
        // Check to see if the command is equal to the quite command (and handle it appropriately if so)
        detectQuitCmd(commandString);
        
        // Initialise a placeholder variable to read the process status number into
        int status = 0;
        
        // Fork a process, or wait for the currently excecuting one to complete...
        if (fork() != 0) {
            // This is the parent process, wait for the child process to complete, read status into status placeholder var...
            waitpid(-1, &status, 0);
            
            // TODO: Check status, provide error info
            if (status != 0) {
                // An error occured...
                
            }
        } else {
            // This is the child process, execute the user's command with any parameters they've passed in...

            // Create an array containing NULL to pass as envp (as per the example I looked up at http://geoffgarside.co.uk/2009/08/28/using-execve-for-the-first-time/ )
            char *envp[] = { NULL };
            
            // Execute the command!
            int execStatus = execve(commandString, parametersArray, envp);
            
            // Did execve fail because of no such file found - if so, try executing the program from the list of places in the PATH environment variable instead...
            if (execStatus == -1) {
                // Was the problem 'No such file or directory' - if so, look in the directories listed in the PATH var for the command instead, try and execute it from there...
                if (errno == ENOENT) {
                    // I looked up getenv() at http://www0.cs.ucl.ac.uk/staff/ucacbbl/getenv/
                    const char *pathVar = getenv("PATH");
                    
                    // Check we have some PATH vars and put them to good use...
                    if (pathVar == NULL) {
                        // No executable containing dirs. in the PATH - warn the user!
                        printf("\nNo PATH environment var - you may wish to list places containing executables in there then try again.\n");
                        
                    } else {
                        // We have a PATH var - a string full of directories to try. Split up the PATH into various directories to try...
                        // Split the PATH by colon string
                        // (I looked up the strtok function at http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm )
                        // A placeholder array to place the executable paths in...
                        // Create a sizeof(char*) placeholder to replace so many calls to sizeof/make code easier to modify in the future if the array ever needed to hold different types...
                        size_t stringSize = PARAMETER_INPUT_LENGTH_MAX * sizeof(char*);
                        char **paths = malloc(stringSize);
                        
                        // We want to separate the string based on colons...
                        const char *seperator = ":";
                        
                        // Start the tokenizer by passing in the buffer string and the space seperator...
                        char *token = strtok(pathVar, seperator);
                        
                        int arraySizeCount = 0;
                        
                        // While there's still tokens to be read in, read them!
                        while (token != NULL) {
                            // it's another path, add it to the paths array and resize
                            // Increment the size counter so the array is resized to the correct new size...
                            arraySizeCount++;
                            
                            // Calculate new size - it must hold the new desired array size number of strings...
                            size_t newArraySize = (arraySizeCount * stringSize);
                            
                            // Realloc the array to make it big enough
                            paths = realloc(paths, newArraySize);
                            
                            // And insert at new index! (which will be the array size - 1)
                            size_t newIndex = (arraySizeCount - 1);
                            
                            // Insert the new path at the new index...
                            paths[newIndex] = token;
                            
                            // And read the next token in...
                            token = strtok(NULL, seperator);
                            
                        }
                        
                        // Now iterate through the paths array, trying each one and appending the command entered onto the end, until one works...
                        for (int i = 0; i < arraySizeCount; i++) {
                            const char *path = paths[i];
                            // Create the new path by concatenating the command the user input on to the end of the path
                            char *newExecPath = malloc(stringSize);
                            
                            // Copy the path into the newExecPath so we can append to it
                            strcpy(newExecPath, path);
                            
                            // Concat a trailing / onto the end...
                            // (I looked up strcat at http://www.tutorialspoint.com/c_standard_library/c_function_strcat.htm )
                            strcat(newExecPath, "/");
                            
                            // Now, concat the command on to the end of the new path
                            strcat(newExecPath, commandString);
                            
                            // Try executing the new comamnd path, with existing params & environment...
                            execStatus = execve(newExecPath, parametersArray, envp);
                            
                            // Free the dynamically allocated path; we're done with it
                            free(newExecPath);
                            
                            // If it went without error, break out of this loop - we only ever want to execute one run of the command!
                            if (execStatus == 0) {
                                // It worked!
                                break;
                            } else {
                                // If it still wasn't found, continue on in this for loop trying the next path - but, if the error was something else, break out and let our default error handling code in the outer loop handle it!
                                 if (errno == ENOENT) {
                                     // Continue, trying the next path in the PATH array
                                     continue;
                                 } else {
                                     // Something else went wrong; handle it!
                                     break;
                                 }
                            }

                            
                        }
                        
                        // Free the paths array
                        free(paths);
                        
                    }

                }
                
            }
            
            // Back to default error handling for all scenarios now.
            // In error scenarios, execve returns -1...
            if (execStatus == -1) {
                // An error occured...
                //Print a human readable representation to the shell
                perror("error");
                
                
            }
            
        }
        
        // Free the command string pointer; we've finished with it and need to clear it from the heap
        free(commandString);
        
    }
    
    return 0;
}


// A function to print the shell's command prompt to the console...
void prompt() {
    // Print current directory...
    printCurrentDirToConsole();
    
    // And print a colon then a space so the user can easily see where they're entering their command...
    printf(": ");
    
}


// A function to read an input from the console, split it into command and parameters, and read these values into the pointers passed into it...
void readCmd(char *cmd, char **params[]) {
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
        // error! Print a human readable representation to the shell
        perror("shell error");
        
        // and exit abruptly...
        exit(1);
    }
    
    // Split the command input line by space string...
    // (I looked up the strtok function at http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm )
    char *inputCommand = malloc(bufferSize * sizeof(char));
    
    // We want to separate the string based on spaces...
    const char *seperator = " ";
    
    // Start the tokenizer by passing in the buffer string and the space seperator...
    char *inputToken = strtok(buffer, seperator);
    
    // The input command will be the first token but has not yet been set - this boolean acts as a flag, so we know when it has been set and do not overwrite it
    bool inputCommandSet = false;
    
    int arraySizeCount = 0;
    
    // While there's still tokens to be read in, read them!
    while (inputToken != NULL) {
        if (inputCommandSet == false) {
            // Copy the token into the input command
            strcpy(inputCommand, inputToken);
            
            // and set the flag
            inputCommandSet = true;
            
        } else {
            // it's a parameter, add it to the parameters array and resize
            // Increment the size counter so the array is resized to the correct new size...
            arraySizeCount++;
            
            // Create a sizeof(char*) placeholder to replace so many calls to sizeof/make code easier to modify in the future if the array ever needed to hold different types...
            size_t stringSize = PARAMETER_INPUT_LENGTH_MAX * sizeof(char*);
            
            // Create a copy of the input token to mutate...
            char *mutableParameter = malloc(stringSize);
            strcpy(mutableParameter, inputToken);
            
            // Calculate new size - it must hold the new desired array size number of strings, plus a NULL terminator...
            size_t newArraySize = (arraySizeCount * stringSize) + sizeof(NULL);
            
            // Realloc the array to make it big enough
            params = realloc(params, newArraySize);
            
            // And insert at new index! (which will be the array size - 1)
            size_t newIndex = (arraySizeCount - 1);
            
            params[newIndex] = &mutableParameter;
            
            
        }
        
        // And read the next token...
        inputToken = strtok(NULL, seperator);
        
    }

    // Remove \n chars from parameters
    for (int i = 0; i < arraySizeCount; i++) {
        // Get the desired string to cleanup from the array
        char *string = *params[i];
        
        // Remove the \n chars present
        string = strtok(string, NEW_LINE_CHAR);
        
        params[i] = &string;
        
    }
    
    // Terminate the parameters array with a NULL
    params[arraySizeCount] = NULL;
    
    
    // Copy the value of the input command string we read into the cmd pointer we've been passed (do not assign directly because inputCommand's a local pointer)
    strcpy(cmd, inputCommand);
    
    // Free local dynamically allocated pointers from the heap!
    free(buffer);
    free(inputToken);
    free(inputCommand);
    
}

// This function detects if the string parameter passed to it is equal to the defined constant quit command, and if it is, then it quits the shell with a normal (0) exit status, and prints to the console to let the user know...
void detectQuitCmd(char *cmd) {
    // If the user enters 'q' at the prompt, exit the shell normally
    if (strcmp(cmd, QUIT_STRING) == 0) {
        // The user entered the quit char - quit normally & inform user!
        printf("\nYou entered '%s' - exiting the shell.\n", QUIT_STRING);
        
        // And quit with exit code 0 (a normal quit)
        exit(0);
    }
    
}

// A function to check if the command passed in is a built-in and if so execute it as a built-in and return true so the calling code knows it's been executed, otherwise, return false so the calling code knows that it has not yet been executed
bool executeBuiltIn(char *cmd, char *params[]) {
    // Define builtins
    const char *cdCommand = "cd";
    const char *pwdCommand = "pwd";
    
    // Check to see if cmd matches built in commands
    if (strcmp(cmd, cdCommand) == 0) {
        // Perform 'cd' built-in!
        builtinCd(params);
        
        return true;
    }
    
    if (strcmp(cmd, pwdCommand) == 0) {
        // Perform 'pwd' built-in!
        builtinPwd();
        
        return true;
    }

    
    // It's not a built-in function, return false to indicate as such
    return false;
}

// A built-in 'cd' function, changes the current directory...
void builtinCd(char *params[]) {
    // Get the new directory to switch to
    char *newDirectory = params[0];

    // Change to it via chdir
    // I looked up chdir at http://pubs.opengroup.org/onlinepubs/9699919799/functions/chdir.html
    chdir(newDirectory);
    
}

// A built-in 'pwd' function, prints the current working directory to the console...
void builtinPwd() {
    // Print current working directory...
    printCurrentDirToConsole();
    
    // And print a new-line to follow...
    printf("\n");

    
}

// A function that prints the current path to the console; intended to be used in the pwd built-in function, and in the prompt function. I wrote this function to minimise code duplication.
void printCurrentDirToConsole() {
    // I looked up the getcwd() function at http://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html
    size_t cwdBufferSize = INPUT_BUFFER_SIZE;
    
    // Allocate a buffer of a reasnoble size to read the current working directory path string into
    char *cwdBuffer = malloc(cwdBufferSize * sizeof(char));
    
    
    // Get the current working directory path into
    getcwd(cwdBuffer, cwdBufferSize);
    
    // Print the current working directory's path to the console
    printf("%s", cwdBuffer);
    
    // Free the dynamically allocated buffer
    free(cwdBuffer);
    
}
