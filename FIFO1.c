#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int pdesk;
    if (mkfifo("pFIFO", 0777) == -1)
    {
        printf("blad\n");
        exit(1);
    };
    switch (fork())
    {
    case -1:
        exit(1);
    case 0:
        fprintf(stderr, "jestem potomny\n");
        close(1);
        pdesk = open("pFIFO", O_WRONLY);
        if (pdesk != 1)
        {
            printf("blad deskryptora do zapisu\n");
            exit(1);
        };
        fprintf(stderr, "robie who\n");
        execlp("who", "who", NULL);
        exit(1);
    default:
        close(0);
        pdesk = open("pFIFO", O_RDONLY);
        if (pdesk != 0)
        {
            printf("blad deskr odczytu\n");
            exit(1);
        };
        printf("robie wc...\n");
        execlp("wc", "wc", "-l", NULL);
        exit(1);
    }
}
