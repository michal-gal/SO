#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid;
    pid_t parent = getpid();

    printf("proces macierzysty: PID: %d, PPID: %d, UID: %d, GID: %d\n",
           (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());

    for (int i = 0; i < 3; i++)
    {
        if (fork() == 0)
        {
            printf("proces potomny:     PID: %d, PPID: %d, UID: %d, GID: %d\n",
                   (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());
        }
    }
    if (getpid() == parent)
    {
        sleep(1); // dajemy czas na utworzenie procesów

        char cmd[256];
        snprintf(cmd, sizeof cmd, "pstree -p %d", (int)parent);
        printf("\nDrzewo procesu %d (pstree):\n", (int)parent);
        system(cmd);
    }
    else
    {
        sleep(5); // procesy potomne czekają, aby prces macierzysty wyswietlil pstree
        exit(0);
    }

    return 0;
}
