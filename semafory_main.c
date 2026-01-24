#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
    int semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0600); // utworzenie zestawu 5 semaforów
    if (semid == -1)                                      // sprawdzenie błędu
    {
        perror("semget");
        exit(1);
    }

    for (int i = 0; i < 5; i++) // inicjalizacja semaforów na 0
        semctl(semid, i, SETVAL, 0);

    pid_t pid;

    for (int i = 0; i < 3; i++) // tworzenie 3 procesów potomnych
    {
        pid = fork();
        if (pid == -1) // sprawdzenie błędu
        {
            perror("fork");
            exit(1);
        }
        if (pid == 0) // proces potomny
        {
            char cmd[256];
            char semid_str[32];
            snprintf(cmd, sizeof cmd, "./p%d", i + 1);
            snprintf(semid_str, sizeof semid_str, "%d", semid);
            execl(cmd, cmd, semid_str, (char *)NULL);
            perror("execl");
            _exit(127);
        }
    }

    for (int i = 0; i < 3; i++) // czekanie na zakończenie wszystkich procesów potomnych
        wait(NULL);

    semctl(semid, 0, IPC_RMID); // usunięcie zestawu semaforów
    return 0;
}
