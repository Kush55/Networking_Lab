# Networking_Lab

# ENSEA Shell

This is a simple command-line shell program implemented in C, designed to simulate basic shell behavior. It allows you to execute commands, manage input/output redirection, and track the execution time of each command. 

## Features
- **Command Execution**: Execute shell commands.
- **Redirection Support**: Supports input (`<`) and output (`>`) redirection.
- **Execution Time**: Tracks and displays the time taken to execute each command in milliseconds.
- **Exit Status**: Displays the exit status of the executed commands or any signal termination.
- **Exit Command**: Type `exit` to quit the shell.

## Requirements
- A Unix-like environment (Linux, macOS) for running the shell.
- GCC compiler for building the program.

## Compilation and Execution

1. **Clone the repository or copy the code** into a file called `enseash.c`.

2. **Compile the program** using GCC:
   ```bash
   gcc -o enseash enseash.c
   ```

3. **Run the shell**:
   ```bash
   ./enseash
   ```

   This will start the ENSEA shell, where you can start typing commands.

## Commands

- **`exit`**: Exit the shell.
- Any other command will be executed as a regular system command.

## Redirection

- **Input Redirection**: Use `<` to redirect input from a file.
  
  Example:
  ```bash
  enseash > input.txt
  ```

- **Output Redirection**: Use `>` to redirect output to a file.

  Example:
  ```bash
  enseash ls > output.txt
  ```

## Exit Status and Time

After each command execution, the prompt displays the exit status and the time taken to execute the command:

```
enseash [exit:0|5ms] %%
```

- **exit:0**: Command exited successfully (0 status).
- **sign:9**: Command terminated by signal (e.g., SIGKILL).
- **5ms**: Time taken for command execution.

