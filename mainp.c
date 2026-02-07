#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <signal.h>

#define P 12    // liczba  procesow producent�w i konsument�w
#define MAX 10  // rozmiar puli bufor�w
#define MAX2 12 // rozmiar pami�ci dzielonej
#define PUSTY 1 // typ komunikatu
#define PELNY 2 // typ komunikatu
// struktura komunikatu
struct bufor
{
   long mtype; // typ komunikatu
   int mvalue; // wartosc komunikatu
};
int shmID, semID, msgID; // ID semafora, kolejki kom. pamieci dzielonej

// funkcja koniec -- do obslugi przerwania
void koniec(int sig)
{
   msgctl(msgID, IPC_RMID, NULL);                             // usuniecie kolejki komunikatow
   shmctl(shmID, IPC_RMID, NULL);                             // usuniecie pamieci dzielonej
   semctl(semID, 0, IPC_RMID, NULL);                          // usuniecie semafora
   printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig); // komunikat
   exit(1);                                                   // zakonczenie programu
}
int main()
{
   key_t klucz, kluczm, kluczs; // klucze do IPC
   int i;
   struct bufor komunikat;             // struktura komunikatu
   struct sigaction act;               // struktura do obslugi sygnalow
   act.sa_handler = koniec;            // ustawienie funkcji obslugi sygnalu na koniec
   sigemptyset(&act.sa_mask);          // wyczyszczenie maski sygnalow
   act.sa_flags = 0;                   // brak dodatkowych flag
   sigaction(SIGINT, &act, 0);         // ustawienie obslugi sygnalu SIGINT
   if ((klucz = ftok(".", 'A')) == -1) // generowanie klucza
   {
      printf("Blad ftok (main)\n"); // komunikat o bledzie
      exit(1);
   }

   msgID = msgget(klucz, IPC_CREAT | IPC_EXCL | 0666); // utworzenie kolejki komunikatow
   if (msgID == -1)                                    // sprawdzenie bledy przy tworzeniu kolejki
   {
      printf("blad kolejki komunikatow\n");
      exit(1);
   }

   kluczm = ftok(".", 'B');                                                 // generowanie klucza do pamieci dzielonej
   shmID = shmget(kluczm, MAX2 * sizeof(int), IPC_CREAT | IPC_EXCL | 0666); // utworzenie pamieci dzielonej
   if (shmID == -1)                                                         // sprawdzenie bledu przy tworzeniu pamieci dzielonej
   {
      printf("blad pamięci dzielonej\n");
      exit(1);
   }

   kluczs = ftok(".", 'C');                                // generowanie klucza do semafora
   semID = semget(kluczs, 2, IPC_CREAT | IPC_EXCL | 0666); // utworzenie 2 semaforów
   if (semID == -1)
   {
      printf("blad semaforów \n");
      exit(1);
   }
   semctl(semID, 0, SETVAL, 1);
   semctl(semID, 1, SETVAL, 1);

   komunikat.mtype = PUSTY; // ustawienie typu komunikatu na PUSTY
   //   komunikat.mvalue=0;
   for (i = 0; i < MAX; i++) // wyslanie MAX komunikatow PUSTY do kolejki
   {
      if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1) // wyslanie komunikatu PUSTY
      {
         printf("blad wyslania kom. pustego\n");
         exit(1);
      };
      printf("wyslany pusty komunikat %d\n", i);
   }
   for (i = 0; i < P; i++) // utworzenie P procesow producentow
      switch (fork())      // tworzenie procesu potomnego
      {
      case -1:
         perror("Blad fork (mainprog)"); // komunikat o bledzie
         exit(2);
      case 0:
         execl("./prod", "prod", NULL); // uruchomienie programu producenta
      }

   for (i = 0; i < P; i++) // utworzenie P procesow konsumentow
      switch (fork())      // tworzenie procesu potomnego
      {
      case -1:
         printf("Blad fork (mainprog)\n");
         exit(2);
      case 0:
         execl("./kons", "kons", NULL);
      }

   for (i = 0; i < 2 * P; i++) // oczekiwanie na zakonczenie wszystkich procesow potomnych
   {
      wait(NULL); // oczekiwanie na zakonczenie procesu potomnego
   }

   msgctl(msgID, IPC_RMID, NULL);    // usuniecie kolejki komunikatow
   shmctl(shmID, IPC_RMID, NULL);    // usuniecie pamieci dzielonej
   semctl(semID, 0, IPC_RMID, NULL); // usuniecie semafora
   printf("MAIN: Koniec.\n");
   return 0;
}
