#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024

int main()
{
    char input[MAX_INPUT_SIZE];

    // Display the welcome message
    printf("Welcome to ENSEA Tiny Shell.\n");
    printf("Type 'exit' to quit.\n");

    while (1) {
            // Display the prompt
        printf("enseash %% ");

        // Read user input
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            // Handle EOF (e.g., user pressed Ctrl+D)
            printf("\nGoodbye!\n");
            break;
        }
                // Remove the newline character at the end of the input
        input[strcspn(input, "\n")] = '\0';

        // Check if the user wants to exit
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        // Fork a child process to execute the command
        pid_t pid = fork();

        if (pid == -1) {
            // Fork failed
            perror("fork");
            continue;
        } else if (pid == 0) {
            // Child process: execute the command
            if (execlp(input, input, NULL) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
                    } else {
            // Parent process: wait for the child to finish
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}
    
