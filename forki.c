#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

1----------------------------------------

int main()
{

printf("START\n");
fork();
printf("META\n");
}

2-------------------------------------


printf("START\n");
execlp("ls", "ls", "-l",NULL);
printf("META\n");

3------------------------------------

printf("START\n");
if(fork()==0)
    execlp("cal", "cal", "1290",NULL);
printf("META\n");

4------------------------------------

printf("START\n");
if(fork()==0)
    execlp("cal", "cal", "1290",NULL);
wait(NULL);
printf("META\n");


5---------------------------------------

int pid1,pid2;

if(pid1=fork()){
	printf("rodzic - pid=%d  ppid=%d pid1=%d\n",getpid(), getppid(),pid1);
	pid2=wait(NULL);
        printf("zakonczyl sie proces %d\n",pid2);
        }
else
       printf("potomek - pid=%d  ppid=%d  pid1=%d\n",getpid(), getppid(), pid1);

6---------------------------------------------------

int pid1,pid2, status;

if(pid1=fork()){
	printf("rodzic - pid=%d  ppid=%d pid1=%d\n",getpid(), getppid(),pid1);
	//sleep(1);
        kill(pid1,9);
        pid2=wait(&status);
        printf("zakonczyl sie proces %d z kodem %d\n",pid2, status);
        }
else
       printf("potomek - pid=%d  ppid=%d  pid1=%d\n",getpid(), getppid(), pid1);

7---------------------------------------------------------

if(fork()==0){
	sleep(20);
	exit(0);
	}
exit(0);

8------------------------------------------------------------------

if(fork()==0)
	exit(0);
sleep(30);
wait(NULL);

9--------------------------------------------------------------------

int main(int argc, char * argv[])
{
FILE *wynik;
close(1);
wynik=fopen("wynik", "r+");

execvp("who", argv);
//argumenty linii polecen przekazane do who

//execlp ("ls", "ls", "-l", NULL);

}