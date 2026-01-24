#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define MAX_FILENAME 256

void perror_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int get_process_limit()
{
    struct rlimit rlim;
    if (getrlimit(RLIMIT_NPROC, &rlim) == -1)
        perror_exit("getrlimit");
    return rlim.rlim_cur;
}

void producer(int fd_write, int num_chars)
{
    char fd_str[16], num_str[16];
    sprintf(fd_str, "%d", fd_write);
    sprintf(num_str, "%d", num_chars);
    execl("./producent", "producent", fd_str, num_str, NULL);
    perror_exit("execl");
}

void consumer(int fd_read)
{
    char fd_str[16];
    sprintf(fd_str, "%d", fd_read);
    execl("./konsument", "konsument", fd_str, NULL);
    perror_exit("execl");
}

long count_chars_in_files(const char *pattern)
{
    DIR *dir = opendir(".");
    if (!dir)
        perror_exit("opendir");
    struct dirent *entry;
    long total = 0;
    while ((entry = readdir(dir)))
    {
        if (strstr(entry->d_name, pattern))
        {
            FILE *fp = fopen(entry->d_name, "r");
            if (!fp)
                continue;
            fseek(fp, 0, SEEK_END);
            total += ftell(fp);
            fclose(fp);
        }
    }
    closedir(dir);
    return total;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <num_kons> <num_prod> <num_chars>\n", argv[0]);
        return 1;
    }

    int num_kons = atoi(argv[1]);
    int num_prod = atoi(argv[2]);
    int num_chars = atoi(argv[3]);

    if (num_kons <= 0 || num_prod <= 0 || num_chars <= 0)
    {
        fprintf(stderr, "Arguments must be positive integers\n");
        return 1;
    }

    int limit = get_process_limit();
    int needed = num_kons + num_prod + 1; // +1 for main
    if (limit < needed)
    {
        fprintf(stderr, "Process limit %d < needed %d\n", limit, needed);
        return 1;
    }

    int fd[2];
    if (pipe(fd) == -1)
        perror_exit("pipe");

    // Fork producers
    for (int i = 0; i < num_prod; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
            perror_exit("fork");
        if (pid == 0)
        {
            close(fd[0]); // close read end
            producer(fd[1], num_chars);
        }
    }

    // Fork consumers
    for (int i = 0; i < num_kons; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
            perror_exit("fork");
        if (pid == 0)
        {
            close(fd[1]); // close write end
            consumer(fd[0]);
        }
    }

    // Close both ends in parent
    close(fd[0]);
    close(fd[1]);

    // Wait for all
    for (int i = 0; i < num_kons + num_prod; i++)
    {
        wait(NULL);
    }

    // Check sums
    long we_sum = count_chars_in_files("we_");
    long wy_sum = count_chars_in_files("wy_");

    printf("we_*.txt total chars: %ld\n", we_sum);
    printf("wy_*.txt total chars: %ld\n", wy_sum);

    if (we_sum == wy_sum)
    {
        printf("Sums match!\n");
    }
    else
    {
        printf("Sums do not match!\n");
    }

    return 0;
}