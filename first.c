#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Welcome to ENSEA Shell.\nType 'exit' to quit.\n");
    while (1) {
        printf("enseash %% ");
        char input[100];
        if (fgets(input, 100, stdin) == NULL) {
            break; // Exit on Ctrl+D
        }

    }
    return 0;
}
