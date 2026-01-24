#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

void perror_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <fd_write> <num_chars>\n", argv[0]);
        return 1;
    }

    int fd_write = atoi(argv[1]);
    int num_chars = atoi(argv[2]);

    char filename[256];
    sprintf(filename, "we_%d.txt", getpid());
    FILE *fp = fopen(filename, "w");
    if (!fp)
        perror_exit("fopen");

    srand(time(NULL) ^ getpid());

    for (int i = 0; i < num_chars; i++)
    {
        char znak = 'A' + (rand() % 26);
        if (fputc(znak, fp) == EOF)
            perror_exit("fputc");
        if (write(fd_write, &znak, 1) != 1)
            perror_exit("write");
    }

    fclose(fp);
    close(fd_write);
    return 0;
}