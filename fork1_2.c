#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t dzieci[3];

    printf("proces macierzysty: PID: %d, PPID: %d, UID: %d, GID: %d\n",
           (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());

    for (int i = 0; i < 3; i++)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            printf("proces potomny:     PID: %d, PPID: %d, UID: %d, GID: %d\n",
                   (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());
            _exit(0);
        }

        dzieci[i] = pid;
    }

    printf("drzewo procesow:\n");
    printf("%d\n", (int)getpid());
    for (int i = 0; i < 3; i++)
        printf("|- %d\n", (int)dzieci[i]);
    return 0;
}