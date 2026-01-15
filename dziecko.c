#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    printf("proces potomny: PID=%d, PPID=%d, UID=%d, GID=%d\n",
           getpid(), getppid(), getuid(), getgid());
    return 0;
}
