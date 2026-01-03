#include <stdio.h>
#include <unistd.h>
// wysokopoziomowo
int main()
{
    int i = 0;
    while (i < 10)
    {
        printf("1");
        sleep(1);
        // fflush(stdout);
        i++;
    }
    i = 0;
    while (i < 10)
    {
        fprintf(stderr, "2");
        sleep(1);
        i++;
    }
}