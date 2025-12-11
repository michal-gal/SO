/*
  Dokumentacja: fork_demo — demonstracja działania fork()

  Krótki opis:
  Program pokazuje tworzenie procesu potomnego przy pomocy fork(),
  rozróżnianie wykonania w rodzicu i dziecku, przekazywanie kodu zakończenia
  z dziecka do rodzica oraz użycie waitpid() do odczytu statusu.

  Kompilacja:
    gcc -Wall -O2 -o fork_demo fork.c

  Uruchomienie:
    ./fork_demo

  Co robi program:
  - Wypisuje PID i wartość zmiennej counter przed fork().
  - Wywołuje fork().
    - W procesie potomnym (pid == 0):
      - zwiększa counter o 10,
      - wypisuje informacje i kończy się przez exit(7).
    - W procesie rodzica (pid > 0):
      - zwiększa counter o 1,
      - wypisuje informacje wraz z PID potomka,
      - oczekuje na potomka przez waitpid(pid, &status, 0) i interpretuje status
        (WIFEXITED/WEXITSTATUS lub WIFSIGNALED/WTERMSIG).

  Obsługa błędów:
  - Błąd fork() -> perror("fork failed"), powrót 1.
  - Błąd waitpid() -> perror("waitpid failed"), powrót 1.

  Eksperymenty:
  - Usuń exit(7) w potomku, sprawdź kod zwracany.
  - Użyj abort() w potomku, zobacz zakończenie sygnałem.
  - Dodaj kolejne fork() aby zbudować drzewo procesów.

  Licencja:
  - Kod demonstracyjny, do użytku edukacyjnego.
*/
// ...existing code...

// /home/michal/programming/SO/fork.c
// Przykład pokazujący, jak działa fork()
// Kompilacja: gcc -Wall -O2 -o fork_demo fork.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(void)
{
    pid_t pid;
    int counter = 0;

    printf("Przed fork(): pid=%d, ppid=%d, counter=%d\n", (int)getpid(), (int)getppid(), counter);

    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        // Kod wykonywany przez proces potomny
        counter += 10;
        printf("[Dziecko]  pid=%d, ppid=%d, fork() zwrocilo %d, counter=%d\n",
               (int)getpid(), (int)getppid(), (int)pid, counter);
        // Możemy zwrócić specyficzny kod wyjścia, żeby rodzic mógł go odczytać
        exit(7);
    }
    else
    {
        // Kod wykonywany przez proces rodzica
        counter += 1;
        printf("[Rodzic]   pid=%d, ppid=%d, fork() zwrocilo %d (PID dziecka), counter=%d\n",
               (int)getpid(), (int)getppid(), (int)pid, counter);

        // Czekamy na zakończenie potomka i odczytujemy status
        int status;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1)
        {
            perror("waitpid failed");
            return 1;
        }

        if (WIFEXITED(status))
        {
            printf("[Rodzic]   Potomek %d zakonczyl sie z kodem %d\n", (int)w, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("[Rodzic]   Potomek %d zostal zabity sygnalem %d\n", (int)w, WTERMSIG(status));
        }
        else
        {
            printf("[Rodzic]   Potomek %d zakonczyl sie w innym stanie\n", (int)w);
        }
    }

    // W obu procesach mozemy dalej wykonywac kod niezaleznie
    printf("Koniec procesu pid=%d, counter=%d\n", (int)getpid(), counter);
    return 0;
}
// ...existing code...