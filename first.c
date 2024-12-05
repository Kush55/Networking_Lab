#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For fork and exec
#include <sys/types.h> 
#include <sys/wait.h> // For wait

int main() {
    printf("Welcome to ENSEA Shell.\nType 'exit' to quit.\n");
    
    while (1) {
        printf("enseash %% ");
        char input[100];
        
        // Read user input
        if (fgets(input, 100, stdin) == NULL) {
            break; // Exit on Ctrl+D
        }
        
        // Remove the newline character
        input[strcspn(input, "\n")] = '\0';
        
        // Check for exit command
        if (strcmp(input, "exit") == 0) {
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
            int status;
            waitpid(pid, &status, 0);
        }
    }
    printf("Goodbye!\n");
    return 0;
}
