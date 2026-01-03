#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Użycie: %s <katalog>\n", argv[0]);
        return 1;
    }

    DIR *dirp;
    struct stat status;
    struct dirent *direntp;

    dirp = opendir(argv[1]);
    if (dirp == NULL)
    {
        printf("Nie można otworzyć katalogu: %s\n", argv[1]);
        return 1;
    }

    while ((direntp = readdir(dirp)) != NULL)
    {
        printf("\t%s\n", direntp->d_name);
        lstat(direntp->d_name, &status);
        printf("czas dostępu=%ld\nrozmiar=%ld\n", status.st_atime, status.st_size);
        if (S_ISDIR(status.st_mode))
            printf("katalog\n");
    }
    closedir(dirp);
}