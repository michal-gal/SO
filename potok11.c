#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int pdesk[2];
    pipe(pdesk);
    switch (fork())
    {
    case -1:
        exit(1);
    case 0:
        close(1);
        dup(pdesk[1]);
        close(pdesk[0]);
        close(pdesk[1]);
        execlp("cat", "cat", "potok1.c", NULL);
    default:
        close(0);
        dup(pdesk[0]);
        close(pdesk[0]);
        close(pdesk[1]);
        execlp("grep", "grep", "close", NULL);
    }
}
