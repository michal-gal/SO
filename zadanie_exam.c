#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t id;
    int i = getpid();
    fork();
    printf("A PID:%d\n", ((int)getpid() - i));
    if ((id = fork()) == 0)
    {
        printf("B PID:%d\n", ((int)getpid() - i));
        fork();
        {
            char pidbuf[16];
            snprintf(pidbuf, sizeof pidbuf, "%d", ((int)getpid() - i));
            execlp("printf", "printf", "C PID:%s\n", pidbuf, (char *)NULL);
        }
    }
    wait(NULL);
    printf("D PID:%d\n", (int)getpid() - i);
    return 0;
}
