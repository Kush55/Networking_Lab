#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_PCB()
{
    pid_t pid = getpid();     // Get process ID
    pid_t ppid = getppid();   // Get parent process ID
    uid_t uid = getuid();     // Get user ID
    gid_t gid = getgid();     // Get group ID

    printf("Process Information:\n");
    printf("PID: %d\n", pid);
    printf("PPID: %d\n", ppid);
    printf("UID: %d\n", uid);
    printf("GID: %d\n", gid);
}

int main()
{
    pid_t ret = fork();
    printf("fork() returned: %d\n", ret);
    print_PCB();
    exit(EXIT_SUCCESS);
}
