
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <fcntl.h>

void P(int semid, int s)
{
    struct sembuf op = {s, -1, 0};
    semop(semid, &op, 1);
}

void V(int semid, int s)
{
    struct sembuf op = {s, +1, 0};
    semop(semid, &op, 1);
}

void sekcja(const char *name)
{
    int fd = open("wynik.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1)
    {
        dprintf(fd, "Sekcja %s procesu o PID=%d\n", name, (int)getpid());
        close(fd);
    }
    printf("Sekcja %s procesu o PID=%d\n", name, (int)getpid());
    sleep(1);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <semid>\n", argv[0]);
        return 1;
    }

    int semid = atoi(argv[1]);

    P(semid, 0); // t21 → t31
    sekcja("t31");
    sekcja("t32");
    V(semid, 1); // t32 → t22
    P(semid, 4); // t23 → t33
    sekcja("t33");

    return 0;
}
