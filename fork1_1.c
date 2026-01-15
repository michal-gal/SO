#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("PID: %d, PPID: %d, UID: %d, GID: %d\n", getpid(), getppid(), getuid(), getgid());
    return 0;
}