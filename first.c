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
    int status = 0;

    while (1) {
        // Display prompt with execution status and time
        static long elapsed_time = 0; // To hold execution time for the previous command
        if (WIFEXITED(status)) {
            printf("enseash [exit:%d|%ldms] %% ", WEXITSTATUS(status), elapsed_time);
        } else if (WIFSIGNALED(status)) {
            printf("enseash [sign:%d|%ldms] %% ", WTERMSIG(status), elapsed_time);
        } else {
            printf("enseash %% ");
        }
        char input[100];
        if (!fgets(input, sizeof(input), stdin)) { // Handle Ctrl+D
            printf("\nBye bye...\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // Remove newline
        if (strcmp(input, "exit") == 0) {  // Handle 'exit'
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

        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Execute command
            execvp(args[0], args);
            perror("Command not found"); // Error handling
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process: Wait and capture status
            wait(&status);
            clock_gettime(CLOCK_MONOTONIC, &end);


            // Calculate elapsed time in milliseconds
            elapsed_time = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
