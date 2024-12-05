#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> // For clock_gettime

#define BILLION 1000000000L

int main() {
    printf("Welcome to ENSEA Shell.\nType 'exit' to quit.\n");
    int status = 0; // To track the exit status of commands
    long elapsed_time = 0; // To track command execution time in ms

    while (1) {
        // Display the prompt with exit status and execution time
        if (WIFEXITED(status)) {
            printf("enseash [exit:%d|%ldms] %% ", WEXITSTATUS(status), elapsed_time);
        }else if (WIFSIGNALED(status)) {
            printf("enseash [sign:%d|%ldms] %% ", WTERMSIG(status), elapsed_time);
        } else {
            printf("enseash %% ");
        }
        // Get user input
        char input[100];
        if (!fgets(input, sizeof(input), stdin)) { // Handle Ctrl+D
            printf("\nBye bye...\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // Remove newline character
        // Check for 'exit' command
        if (strcmp(input, "exit") == 0) {
            printf("Bye bye...\n");
            break;
        }
        // Parse the command and arguments
        char *args[10];
        int arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && arg_count < 10) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the arguments array

        // Measure execution time
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Execute command with arguments
            execvp(args[0], args);
            perror("Command not found"); // Error if command fails
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process: Wait for the child process
            wait(&status);
            clock_gettime(CLOCK_MONOTONIC, &end);

            // Calculate elapsed time in milliseconds
            elapsed_time = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;
        }else {
            perror("Fork failed"); // Error if fork fails
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
