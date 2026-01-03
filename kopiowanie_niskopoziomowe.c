#include <stdio.h>
#include <unistd.h>
// niskopoziomowo bez buforowania
int main()
{
    int i = 0;
    while (i < 10)
    {
        write(1, "1", 1);
        sleep(1);
        i++;
    }
    i = 0;
    while (i < 10)
    {
        write(2, "2", 1);
        sleep(1);
        i++;
    }
}