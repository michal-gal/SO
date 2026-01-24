#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

// maksymalny rozmiar wiadomosci
#define MAX 80
#define SERWER 1 // typ komunikatu do serwera
char temp[15];
static int IDkolejki_global = -1;
static int is_owner = 0;

int wezpid(char text[]); // odbiera pid procesu z tresci komunikatu
void sig_hand(int);
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

    int msize;
    int i, pid;
    key_t key;            // unikalny klucz kolejki komunikatow
    int IDkolejki;        // identyfikator kolejki
    struct komunikat kom; // przesylany komunikat
    // tworzenie unikalnego klucza urzadzenia IPC dla kolejki komunikatow
    key = ftok(".", 98);

    // Wielu serwerów: pierwszy tworzy kolejkę, reszta tylko się podłącza.
    is_owner = 0;
    IDkolejki = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (IDkolejki == -1)
    {
        if (errno == EEXIST)
        {
            IDkolejki = msgget(key, 0666);
        }
    }
    else
    {
        is_owner = 1;
    }

    if (IDkolejki == -1)
    {
        perror("msgget");
        exit(1);
    }

    IDkolejki_global = IDkolejki;
    signal(SIGINT, sig_hand); // po nacisnieciu przez uzytkownika CTRL+C wywoluje sie funkcja sig_hand()
    signal(SIGTERM, sig_hand);

    if (is_owner)
    {
        printf("S: uruchomiono jako OWNER (twórca kolejki)\n");
    }
    else
    {
        printf("S: uruchomiono jako WORKER (podłączony do kolejki)\n");
    }
    printf("^C konczy prace serwera\a\n");
    sleep(1);
    printf("\a");
    while (1)
    {
        printf("S: Czekam na komunikat...\n");
        kom.mtype = SERWER; // odczytuje z kolejki komunikat typu 1
        if (msgrcv(IDkolejki, (struct msgbuf *)&kom, MAX, kom.mtype, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("S: Odebrano od: %s\n", kom.mtext);
        // przetwarzamy wiadomosc
        msize = strlen(kom.mtext);
        // text = malloc(sizeof(char) * msize);
        for (i = 0; i < msize; i++)
        {
            kom.mtext[i] = toupper(kom.mtext[i]);
        }
        pid = wezpid(kom.mtext);
        // wysylanie wiadomosci
        kom.mtype = pid;
        printf("S: Wysylanie... %s -> %ld\n", kom.mtext, kom.mtype);
        if (msgsnd(IDkolejki, (struct msgbuf *)&kom, strlen(kom.mtext) + 1, 0) == -1)
        {
            perror("msgsnd");
            exit(1);
        }
    }
}
int wezpid(char text[])
{
    int i, pid, len, oldi;
    len = strlen(text);
    for (i = 0; i < 12; i++)
    {
        temp[i] = text[i];
        if (temp[i] == '~')
        {
            temp[i + 1] = '\n';
            break;
        }
    }
    oldi = i;
    for (i = 0; i < len - oldi; i++)
    {
        text[i] = text[i + 1 + oldi];
    }
    pid = atoi(temp);
    return pid;
}
void sig_hand(int sig_n)
{
    if ((sig_n == SIGTERM) || (sig_n == SIGINT))
    {
        printf("SIGINT\n");
        if (is_owner && IDkolejki_global != -1)
        {
            msgctl(IDkolejki_global, IPC_RMID, 0);
        }
        exit(0);
    }
}
