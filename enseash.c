#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h> // For open()

#define BILLION 1000000000L

int main() {
    printf("Welcome to ENSEA Shell.\nType 'exit' to quit.\n");
    int status = 0; // To track the exit status of commands
    long elapsed_time = 0; // To track command execution time in ms

    while (1) {
        // Display the prompt with exit status and execution time
        if (WIFEXITED(status)) {
            printf("enseash [exit:%d|%ldms] %% ", WEXITSTATUS(status), elapsed_time);
        } else if (WIFSIGNALED(status)) {
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
        char *input_file = NULL, *output_file = NULL;

        while (token != NULL && arg_count < 10) {
            // Check for input redirection ('<')
            if (strcmp(token, "<") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    input_file = token; // Capture input file
                }
            }
            // Check for output redirection ('>')
            else if (strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    output_file = token; // Capture output file
                }
            }else {
                args[arg_count++] = token; // Regular argument
            }
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the arguments array

        // Measure execution time
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Handle redirections
            if (input_file) {
                int fd_in = open(input_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("Failed to open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO); // Redirect stdin
                close(fd_in);
            }
            if (output_file) {
                int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO); // Redirect stdout
                close(fd_out);
            }

            // Execute command
            execvp(args[0], args);
            perror("Command not found"); // Error if command fails
            exit(EXIT_FAILURE);
        }else if (pid > 0) {
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
