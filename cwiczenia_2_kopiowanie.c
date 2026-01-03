#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>

// zmienna do tworzenia pliku

int stworz_plik()
{
    int we;
    we = open("we", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    for (int i = 0; i < 10000000; i++)
    {
        write(we, "1", 1);
    }
    close(we);
}

// Program kopiujący znak po znaku z wykorzystaniem funkcji niskopoziomowych
int kopiowanie_1()
{
    char c;
    int we, wy;
    we = open("we", O_RDONLY);
    wy = open("wy", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    while (read(we, &c, 1) == 1)
        write(wy, &c, 1);
    gettimeofday(&end, NULL);
    long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    printf("Czas kopiowania niskopoziomowego znak po znaku: %ld ms\n", ms);
}

// Program kopiujący znak po znaku z wykorzystaniem funkcji wysokopoziomowych

int kopiowanie_2()
{
    FILE *we, *wy;
    int c;
    we = fopen("we", "r");
    wy = fopen("wy", "w");
    if ((we != NULL) && (wy != NULL))
    {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        while ((c = fgetc(we)) != EOF)
            fputc(c, wy);
        gettimeofday(&end, NULL);
        long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        printf("Czas kopiowania wysokopoziomowego znak po znaku: %ld ms\n", ms);
    }
    else
        printf("blad otwarcia\n");
}

// Program kopiujący blokami o rozmiarze 1024B z wykorzystaniem funkcji niskopoziomowych

int kopiowanie_3(int wielkosc_bloku)
{
    char blok[wielkosc_bloku];
    int we, wy;
    int liczyt;
    we = open("we", O_RDONLY);
    wy = open("wy", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    while ((liczyt = read(we, blok, sizeof(blok))) > 0)
        write(wy, blok, liczyt);
    gettimeofday(&end, NULL);
    long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    printf("Czas kopiowania niskopoziomowego blokami %dB: %ld ms\n", wielkosc_bloku, ms);
}

// Program kopiujący blokami o rozmiarze 1024B z wykorzystaniem funkcji wysokopoziomowych
int kopiowanie_4(int wielkosc_bloku)
{
    char blok[wielkosc_bloku];
    FILE *we, *wy;
    int c;
    we = fopen("we", "r");
    wy = fopen("wy", "w");
    if ((we != NULL) && (wy != NULL))
    {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        while (fgets(blok, 1024, we))
            fputs(blok, wy);
        gettimeofday(&end, NULL);
        long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        printf("Czas kopiowania wysokopoziomowego blokami %dB : %ld ms\n", wielkosc_bloku, ms);
    }
    else
        printf("blad otwarcia\n");
}

int main()
{
    stworz_plik();
    kopiowanie_1();
    kopiowanie_2();
    kopiowanie_3(1024);
    kopiowanie_3(2048);
    kopiowanie_3(4096);
    kopiowanie_3(8192);
    kopiowanie_3(16384);
    kopiowanie_4(1024);
    kopiowanie_4(2048);
    kopiowanie_4(4096);
    kopiowanie_4(8192);
    kopiowanie_4(16384);
    return 0;
}