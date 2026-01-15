#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t dzieci[3];

    printf("macierzysty proces PID: %d, PPID: %d, UID: %d, GID: %d\n",
           (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());

    for (int i = 0; i < 3; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0)
        {
            execl("./dziecko", "dziecko", (char *)NULL);
            perror("execl");
            _exit(127);
        }

        dzieci[i] = pid;
    }

    printf("drzewo procesow:\n");
    printf("%d\n", (int)getpid());
    for (int i = 0; i < 3; i++)
        printf("|- %d\n", (int)dzieci[i]);

    int status;
    pid_t dziecko_pid;

    while ((dziecko_pid = wait(&status)) > 0)
    {
        if (WIFEXITED(status))
        {
            printf("Rodzic: proces %d zakonczyl sie kodem %d\n",
                   dziecko_pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Rodzic: proces %d zakonczyl sie sygnalem %d\n",
                   dziecko_pid, WTERMSIG(status));
        }
        else
        {
            printf("Rodzic: proces %d zakonczyl sie niepoprawnie\n",
                   dziecko_pid);
        }
    }

    if (dziecko_pid < 0 && errno != ECHILD)
        perror("wait");

    printf("Rodzic: wszystkie procesy potomne zakonczone\n");
    return 0;
}
