#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main()
{
    int fd;
    char buffer[256];

    printf("UID rzeczywisty: %d\n", getuid());  // Real user ID,     not affected by SUID
    printf("UID efektywny:   %d\n", geteuid()); // Effective user ID, affected by SUID

    fd = open("dane", O_RDWR | O_CREAT, 0600); // Plik do testu zapisu/odczytu
    if (fd < 0)
    { // Sprawdzenie błędu otwarcia pliku
        perror("Błąd open");
        return 1;
    }

    const char *tekst = "Test SUID - zapis danych\n"; // Tekst do zapisania
    write(fd, tekst, strlen(tekst));                  // Zapis danych do pliku

    lseek(fd, 0, SEEK_SET);                       // Przesunięcie wskaźnika pliku na początek
    int n = read(fd, buffer, sizeof(buffer) - 1); // Odczyt danych z pliku
    buffer[n] = '\0';                             // Null-terminacja odczytanych danych

    printf("Odczytane dane: %s", buffer); // Wyświetlenie odczytanych danych

    close(fd);
    return 0;
}
