#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t parent = getpid();

    printf("macierzysty proces PID: %d, PPID: %d, UID: %d, GID: %d\n",
           (int)getpid(), (int)getppid(), (int)getuid(), (int)getgid());

    for (int i = 0; i < 3; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("blad fork-a");
            exit(1);
        }

        if (pid == 0)
        {
            execl("./dziecko", "dziecko", (char *)NULL);
            perror("execl");
            _exit(127);
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

    int status;        // status zakonczenia procesu potomnego
    pid_t dziecko_pid; // PID procesu potomnego

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

    if (dziecko_pid < 0 && errno != ECHILD) // Blad inny niz "brak procesow potomnych"
        perror("wait");                     // wypisz blad

    printf("Rodzic: wszystkie procesy potomne zakonczone\n");
    return 0;
}
