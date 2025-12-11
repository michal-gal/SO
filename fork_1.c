#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int id = fork();
    fork();
    if (id == 0)
    {
        printf("Hello, World! from child process id = %d\n", id);
    }
    else

    {
        printf("Hello, World! from parent process id = %d \n", id);
    }
    // printf("Hello, World! from id = %d\n", id);
    return 0;
}