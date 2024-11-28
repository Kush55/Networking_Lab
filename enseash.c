#include <stdio.h>
#include <string.h>

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

        // Placeholder for future command execution
        printf("Command not implemented: %s\n", input);
    }

    return 0;
}
