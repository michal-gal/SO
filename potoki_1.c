#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int potok[2];          // deskryptory potoku
    if (pipe(potok) == -1) // utworzenie potoku
    {
        perror("pipe error");
        exit(1);
    }

    switch (fork()) // pierwszy fork
    {
    case -1:
        perror("1st fork error");
        exit(1);
    case 0:
        int pdesk;
        if (mkfifo("pFIFO", 0777) == -1 && errno != EEXIST)
        {
            printf("blad\n");
            exit(1);
        };
        switch (fork()) // drugi fork
        {
        case -1:
            exit(1);
        case 0:
            close(potok[0]);
            close(potok[1]);
            pdesk = open("pFIFO", O_WRONLY);
            if (pdesk == -1)
            {
                printf("blad deskryptora do zapisu\n");
                exit(1);
            }
            dup2(pdesk, 1);
            close(pdesk);
            execlp("cat", "cat", "potoki_1.c", NULL);
            exit(1);
        default:
            close(potok[0]);
            pdesk = open("pFIFO", O_RDONLY);
            if (pdesk == -1)
            {
                printf("blad deskr odczytu\n");
                exit(1);
            }
            dup2(pdesk, 0);
            close(pdesk);
            dup2(potok[1], 1);
            close(potok[1]);
            execlp("grep", "grep", "close", NULL);
            exit(1);
        }
    default: // rodzoc
    {
        close(potok[1]);
        dup2(potok[0], 0);
        close(potok[0]);
        unlink("pFIFO");
        execlp("grep", "grep", "pdesk", NULL);
        perror("execlp error");
        exit(2);
    }
    }
}