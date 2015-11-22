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

// The maximum parameter input lenght in chars
#define PARAMETER_INPUT_LENGTH_MAX 255

// Declare the string that quits the shell as a constant
const char *QUIT_STRING = "q";

// Declare the newline char. as a constant to improve code readability and make it easy to change for example if this code is used on a non-standard system with a different newline char to \n
const char *NEW_LINE_CHAR = "\n";

// Declare function prototypes
void prompt();
void readCmd(char *cmd, char **params[]);
void detectQuitCmd(char *cmd);
bool isNewLine(char *cmd);

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
        if (isNewLine(commandString)) {
            // The user just hit return; loop around, don't try to execve a newline char...
            continue;
        }
        
        // Now that we're certain they entered a command, remove the trailing \n from the command string (if one exists)...
        commandString = strtok(commandString, NEW_LINE_CHAR);
        
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
            execve(commandString, parametersArray, NULL);
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
    
    // Split the line by string...
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
            // (inserting the string via strcpy so that a local pointer does not get inserted into an array that lives on beyond this method's scope!)
            size_t newIndex = (arraySizeCount - 1);
            
            params[newIndex] = &mutableParameter;
            
            
        }
        
        // And read the next token...
        inputToken = strtok(NULL, seperator);
        
    }

    // TODO: Remove \n chars from parameters
    for (int i = 0; i < arraySizeCount; i++) {
        // Get the desired string to cleanup from the array
        char *string = *params[i];
        
        // Remove the \n chars present
        string = strtok(string, NEW_LINE_CHAR);
        
        params[i] = &string;
        //strcpy(*params[i], &string);
        
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

// This function checks to see if the cmd string it has been passed is just a new line. It returns true if it is, false if not
bool isNewLine(char *cmd) {
    // If the string comparison returns an offset of 0, then the cmd string is equal to the newline char.
    // (I didn't want to just return a boolean expression from this function as that wouldn't strictly by an stdbool boolean).
    if (strcmp(cmd, NEW_LINE_CHAR) == 0) {
        return true;
    } else {
        return false;
    }
    
}

