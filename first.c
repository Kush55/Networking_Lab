#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For fork and exec
#include <sys/types.h> 
#include <sys/wait.h> // For wait
int main() {
    printf("Welcome to ENSEA Shell.\nType 'exit' to quit.\n");

    int last_status = 0; // Variable to store the status of the last command

    while (1) {
		// Print the dynamic prompt based on the last command's status
        if (WIFEXITED(last_status)) {
            printf("enseash [exit:%d] %% ", WEXITSTATUS(last_status));
        } else if (WIFSIGNALED(last_status)) {
            printf("enseash [sign:%d] %% ", WTERMSIG(last_status));
        } else {
            printf("enseash %% ");
        }
                char input[100];

        // Read user input
        if (fgets(input, 100, stdin) == NULL) { 
            // Handle <Ctrl>+D
            printf("\nBye bye...\n");
            break;
        }
                // Remove the newline character
        input[strcspn(input, "\n")] = '\0';

        // Check for exit command
        if (strcmp(input, "exit") == 0) { 
            printf("Bye bye...\n");
            break;
        }
                // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            // In the child process: execute the command
            execlp(input, input, NULL);
            perror("execlp"); // If execlp fails
            exit(EXIT_FAILURE);
        } else {
            // In the parent process: wait for the child to finish
            waitpid(pid, &last_status, 0);
        }
    }
    
    return 0;
}
