#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void perror_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <fd_read>\n", argv[0]);
        return 1;
    }

    int fd_read = atoi(argv[1]);

    char filename[256];
    sprintf(filename, "wy_%d.txt", getpid());
    FILE *fp = fopen(filename, "w");
    if (!fp)
        perror_exit("fopen");

    char znak;
    while (read(fd_read, &znak, 1) > 0)
    {
        if (fputc(znak, fp) == EOF)
            perror_exit("fputc");
    }

    fclose(fp);
    close(fd_read);
    return 0;
}