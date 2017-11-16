/*-
 * All rights reserved.
 * Name: Anshuman Biswal
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <sys/uio.h>
# include <time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define BUFLEN 100
#define LOG(lf, region) fprintf(lf, "%s: %d: %s\n", getuid(), (msg))
int debug=0;

/*unsigned long getCustomerID()
{
  unsigned long number;
  srand(getpid());
  number = random() % 7100175000;
  printf("%010lu\n",number);
  return number;
}*/

static int nchildren;
static pid_t *pids;

unsigned long getrandom(int start,int end)
{
  unsigned long number;
   number= rand() % (end - start + 1) + start;
  printf("%lu\n",number);
  return number;
}

void generatebill(FILE* billog,char* regionname)
{
time_t t;
/* Intializes random number generator */
   srand((unsigned) time(&t));

	fprintf(billog,"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t %s Bill\n",regionname);
	fprintf(billog,"-------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(billog,"\t\t     Customerd id \t\t\t\t\t\t\t\t\t\t\t Current Bill \t\t\t\t\t\t\t\t\t\t\t\t\t Amount Due\n");
	fprintf(billog,"\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t\t\t\t %lu",getrandom(1000000000,9999999999),getrandom(1000,9999),getrandom(1000,9999));
fprintf(billog,"\n\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t\t\t\t %lu",getrandom(1000000000,9999999999),getrandom(1000,9999),getrandom(1000,9999));
fprintf(billog,"\n\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t %lu \t\t\t\t\t\t\t\t\t\t\t\t\t\t\t %lu",getrandom(1000000000,9999999999),getrandom(1000,9999),getrandom(1000,9999));

fclose(billog);
} 



int main(int argc, char **argv)
{
   int     listenfd, i;
      socklen_t addrlen;
      void    sig_int(int);
    pid_t   child_make(int, int, int);

    if (argc == 3)
         listenfd = listen(NULL, argv[1], &addrlen);
     else if (argc == 4)
         listenfd = listen(argv[1], argv[2], &addrlen);
     else
         printf("usage: %s [ <host> ] <port#> <#children>",argv[0]);
     nchildren = atoi(argv[argc - 1]);
    pids = calloc(nchildren, sizeof(pid_t));

    for (i = 0; i < nchildren; i++)
         pids[i] = child_make(i, listenfd, addrlen); /* parent returns */

     Signal(SIGINT, sig_int);

     for ( ; ; )
         pause();      
}

 void sig_int(int signo)
 {
    int     i;
     void    pr_cpu_time(void);

         /* terminate all children */
     for (i = 0; i < nchildren; i++)
         kill(pids[i], SIGTERM);
     while (wait(NULL) > 0);   /* wait for all children */
      exit(0);
 }


pid_t child_make(int i, int listenfd, int addrlen)
 {
     pid_t   pid;
      void    child_main(int, int, int);

     if ( (pid = fork()) > 0)
          return (pid);            /* parent */

      child_main(i, listenfd, addrlen);     /* never returns */
 }

 void child_main(int i, int listenfd, int addrlen)
{
    int     connfd;
     void    generatebill(FILE *,char *);
     socklen_t clilen;
     struct sockaddr *cliaddr;
     FILE *file = fopen("bill.log", "a+");
     cliaddr = malloc(addrlen);

     printf("child %ld starting\n", (long) getpid());
     for ( ; ; ) {
         clilen = addrlen;
         connfd = accept(listenfd, cliaddr, &clilen);

         generatebill(file,"delhi")     /* process the request */
        Close(connfd);
     }
 }



