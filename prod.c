#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sem.h>

struct bufor
{
   long mtype;
   int mvalue;
};

int *pam;

#define MAX 10
#define MAX2 12
#define PELNY 2
#define PUSTY 1
#define odczyt pam[MAX]
#define zapis pam[MAX + 1]
int main()
{
   key_t klucz, kluczm, kluczs; // klucze do IPC
   int msgID;                   // selektor kolejki komunikatow
   int shmID, semID;            // ID pamieci dzielonej i semafora
   time_t czas;                 // zmienna do inicjalizacji generatora liczb losowych
   struct bufor komunikat;

   srand((unsigned int)(time(&czas) + getpid()));

   if ((klucz = ftok(".", 'A')) == -1) // generowanie klucza do kolejki komunikatow
   {
      printf("Blad ftok (A)\n");
      exit(2);
   };

   msgID = msgget(klucz, IPC_CREAT | 0666); // uzyskanie dostepu do kolejki komunikatow
   if (msgID == -1)
   {
      printf("blad klejki komunikatow\n");
      exit(1);
   }
   kluczm = ftok(".", 'B');                                      // generowanie klucza do pamieci dzielonej
   kluczs = ftok(".", 'C');                                      // generowanie klucza do semafora
   shmID = shmget(kluczm, MAX2 * sizeof(int), IPC_CREAT | 0666); // uzyskanie dostepu do pamieci dzielonej
   if (shmID == -1)
   {
      perror("shmget");
      exit(1);
   }

   pam = (int *)shmat(shmID, NULL, 0);
   if (pam == (void *)-1)
   {
      perror("shmat");
      exit(1);
   }

   // odbiokomunikatu
   if (msgrcv(msgID, &komunikat, sizeof(komunikat.mvalue), PUSTY, 0) == -1)
   {
      perror("msgrcv PUSTY");
      exit(1);
   }

   semID = semget(kluczs, 1, IPC_CREAT | 0666);
   if (semID == -1)
   {
      perror("semget");
      exit(1);
   }

   sleep(getpid() % 10);
   struct sembuf operacje;

   // sekcja krytyczna: zapis PID do bufora pod indeksem "zapis"
   operacje.sem_num = 0;
   operacje.sem_flg = 0;

   operacje.sem_op = -1;
   if (semop(semID, &operacje, 1) == -1)
   {
      perror("semop P");
      exit(1);
   }

   int idx = zapis;
   int pid = (int)getpid();
   pam[idx] = pid;
   printf("PROD pid=%d: zapis do bufora[%d], odczyt=%d, zapis=%d\n", pid, idx, odczyt, zapis);
   zapis = (zapis + 1) % MAX;

   operacje.sem_op = 1;
   if (semop(semID, &operacje, 1) == -1)
   {
      perror("semop V");
      exit(1);
   }

   // wysłanie komunikatu "PELNY" (pojawił się element do odebrania)
   komunikat.mtype = PELNY;
   komunikat.mvalue = pid;
   if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1)
   {
      perror("msgsnd PELNY");
      exit(1);
   }

   shmdt(pam);
   return 0;
}
