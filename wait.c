#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(void)
{
    int id = fork();
    int n;

    if (id == 0)
    {
        n = 1;
    }
    else
    {
        n = 6;
    };
    if (id != 0)
    {
        int status;
        wait(&status);
    }
    int i;
    for (i = 0; i < n; i++)
    {
        printf("Hello, World! from child process id = %d\n", id);
    }
    return 0;
}