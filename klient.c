#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

// maksymalny rozmiar wiadomosci
#define MAX 80
#define SERWER 1 // typ komunikatu wysylanego do serwera
// struktura komunikatu
struct komunikat
{
    long mtype;
    char mtext[MAX];
};
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int nr_klienta, i;
    char tmptxt[10], wiadomosc[80];
    key_t key;            // unikalny klucz kolejki komunikatow
    int IDkolejki;        // identyfikator kolejki
    struct komunikat kom; // przesylany komunikat
    nr_klienta = getpid();
    // tworzenie unikalnego klucza urzadzenia IPC dla kolejki komunikatow
    key = ftok(".", 98);
    // tworzenie kolejki
    if ((IDkolejki = msgget(key, IPC_CREAT | 0660)) == -1)
    {
        perror("msgget() blad kolejki");
        exit(1);
    }
    while (1)
    {
        // wysylanie wiadomosci
        // zapisujemy typ komunikatu - klienci wszyscy wysylaja do serwera komunikat  typu 1
        sprintf(tmptxt, "%d~", getpid()); // postac komunikatu: PID~tekst
        strcpy(kom.mtext, tmptxt);
        printf("K[%d]: Podaj tekst do wyslania:\n", getpid());
        i = 0;
        while (1)
        {
            int c = getchar();
            if (c == EOF)
            {
                printf("K[%d]: EOF, koniec.\n", nr_klienta);
                return 0;
            }
            wiadomosc[i] = (char)c;
            if ((wiadomosc[i] == '\n') || (i >= (MAX - 1)))
            {
                wiadomosc[i] = '\0';
                break;
            }
            i++;
        }
        strcat(kom.mtext, wiadomosc); // laczymy wprowadzona przez uzytkownika wiadomosc do wyslania z zawartoscia kom.text

        printf("K[%d]: Wysylanie... \"%s\" -> SERWER\n", nr_klienta, &kom.mtext[strlen(kom.mtext) - strlen(wiadomosc)]);

        // wyslanie komunikatu do serwera (typ 1)
        kom.mtype = SERWER;
        if (msgsnd(IDkolejki, (struct msgbuf *)&kom, strlen(kom.mtext) + 1, 0) == -1)
        {
            perror("msgsnd");
            exit(1);
        }

        // odczyt odpowiedzi z kolejki: typ = PID klienta
        if (msgrcv(IDkolejki, (struct msgbuf *)&kom, MAX, nr_klienta, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("K[%d]: Odebrano: \"%s\" zaadresowane do %ld\n", nr_klienta, kom.mtext, kom.mtype);
    }
}
