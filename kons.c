#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
struct bufor
{
   long mtype;
   int mvalue;
};
int *pam;
#define MAX2 12
#define MAX 10
#define PELNY 2
#define PUSTY 1
#define zapis pam[MAX + 1]
#define odczyt pam[MAX]

int main()
{
   key_t klucz, kluczm, kluczs; // klucze do IPC
   int msgID, shmID, semID;     // ID pamieci dzielonej, semafora i kolejki komunikatow
   struct bufor komunikat;      // struktura komunikatu
   sleep(1);
   printf("konsument--------------------------------\n");
   if ((klucz = ftok(".", 'A')) == -1)
   {
      printf("Blad ftok (A)\n");
      exit(2);
   };
   msgID = msgget(klucz, IPC_CREAT | 0666); // uzyskanie dostepu do kolejki komunikatow
   if (msgID == -1)
   {
      printf("blad klejki komunikatow\n");
      exit(1);
   };

   kluczm = ftok(".", 'B');                     // generowanie klucza do pamieci dzielonej
   kluczs = ftok(".", 'C');                     // generowanie klucza do semafora
   semID = semget(kluczs, 1, IPC_CREAT | 0666); // uzyskanie dostepu do semafora
   if (semID == -1)
   {
      perror("semget");
      exit(1);
   }

   shmID = shmget(kluczm, MAX2 * sizeof(int), IPC_CREAT | 0666); // uzyskanie dostepu do pamieci dzielonej
   if (shmID == -1)
   {
      perror("shmget");
      exit(1);
   }

   pam = (int *)shmat(shmID, NULL, 0); // dolaczenie pamieci dzielonej
   if (pam == (void *)-1)
   {
      perror("shmat");
      exit(1);
   }

   // odbiór komunikatu "PELNY" (jest element do pobrania)
   if (msgrcv(msgID, &komunikat, sizeof(komunikat.mvalue), PELNY, 0) == -1)
   {
      perror("msgrcv PELNY");
      exit(1);
   }

   sleep(getpid() % 10); // symulacja czasu przetwarzania
   struct sembuf operacje;

   // sekcja krytyczna: odczyt z bufora spod indeksu "odczyt"
   operacje.sem_num = 0; // numer semafora
   operacje.sem_flg = 0; // brak dodatkowych flag

   operacje.sem_op = -1;                 // operacja P (czekanie)
   if (semop(semID, &operacje, 1) == -1) // wykonanie operacji na semaforze
   {
      perror("semop P");
      exit(1);
   }

   int idx = odczyt;     // indeks do odczytu
   int value = pam[idx]; // odczyt wartości z bufora
   printf("KONS pid=%d: odczyt z bufora[%d]=%d, odczyt=%d, zapis=%d\n", (int)getpid(), idx, value, odczyt, zapis);
   odczyt = (odczyt + 1) % MAX;

   operacje.sem_op = 1;                  // operacja V (sygnalizacja)
   if (semop(semID, &operacje, 1) == -1) // wykonanie operacji na semaforze
   {
      perror("semop V");
      exit(1);
   }

   // wysłanie komunikatu "PUSTY" (zwolniło się miejsce)
   komunikat.mtype = PUSTY;                                          // ustawienie typu komunikatu na PUSTY
   komunikat.mvalue = value;                                         // przekazanie odczytanej wartości
   if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1) // wysłanie komunikatu PUSTY
   {
      perror("msgsnd PUSTY");
      exit(1);
   }

   shmdt(pam); // odłączenie pamięci dzielonej
   return 0;
}
