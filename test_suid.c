#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main() {
    int fd;
    char buffer[256];

    printf("UID rzeczywisty: %d\n", getuid());
    printf("UID efektywny:   %d\n", geteuid());

    fd = open("dane", O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        perror("Błąd open");
        return 1;
    }

    const char *tekst = "Test SUID - zapis danych\n";
    write(fd, tekst, strlen(tekst));

    lseek(fd, 0, SEEK_SET);
    int n = read(fd, buffer, sizeof(buffer) - 1);
    buffer[n] = '\0';

    printf("Odczytane dane: %s", buffer);

    close(fd);
    return 0;
}
