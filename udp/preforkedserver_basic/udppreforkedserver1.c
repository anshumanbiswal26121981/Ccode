/* Anshuman Biswal
* All rights reserved.
* Name: Anshuman Biswal
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
# include <stdlib.h>
# include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/wait.h>


#define BUFLEN 1000
#define NPACK 1000
#define PORT 9930
#define NCHILDREN 10
#define LOG(lf, msg) fprintf(lf, "\n child:%ld user:%d: %s\n",(long)getpid(), getuid(), (msg))

#define DEBUG 1
void diep(char *s)
{
	perror(s);
	exit(1);
}
struct clientinfo
{
 char name[BUFLEN];
 struct timespec start;
};
FILE *logfile; 
FILE *avgtimelog,*avgwaittimelog;
FILE *bill;
static pid_t *pids;
double timedelayInSec,totalwaittimeinsec;
double avgtimedelay,avgwaittime;
//struct timeval start,end;

int     listenfd, nthreads;

struct sockaddr_in si_me, si_other;
 /* Declare shared memory variables */
  key_t key,key1;
  int shmid,shmid1;
  double *totaltime;
  double *totalwaittime;
 /* Declare semaphore variables */
  sem_t sem,sem1;
void die(char *s)
{
	perror(s);
	exit(1);
}

unsigned long int getrandom(int start,int end)
{	unsigned long int number;
	number= rand() % (end - start + 1) + start;
	return number;
}

char* timestring ( void )
{
# define TIME_SIZE 60

	const struct tm *tm;
	size_t len;
	time_t now;
	char *s;
	char buffer[60];
	struct timeval curTime;
	double temptime;
	gettimeofday(&curTime, NULL);
	int milli = curTime.tv_usec / 1000;

	now = time ( NULL );
	tm = localtime ( &now );

	s = malloc ( TIME_SIZE * sizeof ( char ) );

	len = strftime (buffer, TIME_SIZE, "%d %B %Y %H:%M:%S", tm );
	sprintf(s,"%s:%d",buffer,milli);

	return s;
# undef TIME_SIZE
}


double timeit(struct timespec start, struct timespec end)
{	
	double elapsesec;
	long elapsensec;
	double result;
	elapsesec = difftime(end.tv_sec,start.tv_sec);
	elapsensec = end.tv_nsec-start.tv_nsec;
	result = elapsesec + ((double)elapsensec)/1.0e9;
	return result;
}


void generatebill(int s)
{	time_t t;
	int count=0;
	size_t bytesSent;
	char buf[BUFLEN];
	struct rusage ru;
	struct clientinfo cli;
	char logBuffer [5120];
	char *c_timestring;
	char timestr[BUFLEN];
	
	double temptotaltime;
	double temptime;
	int slen=sizeof(si_other);
	void killallforkedchild();
	struct timespec start,end;
	//FILE *timeproc = fopen("proctime.log","a+");
	
	logfile=fopen("udp_assignment_server.log", "a+");
	
	while(1) 
	{
		
		
		memset(buf,0,sizeof(buf));
		memset((char*) &cli, 0, sizeof(cli));
		
		if (recvfrom(s,(struct clientinfo *)&cli, (int)sizeof(cli), 0, &si_other, &slen)==-1)
		{
			diep("recvfrom()");
		}else
		{	
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);			
			
			
			strcpy(buf,cli.name);
			printf ("\n %ld child, messgae receivedv = %s ", (long)getpid(),buf);
			fflush(stdout);
			c_timestring = timestring();	
			if(DEBUG)
			{
				printf ("\n messgae receivedv = %s ", buf);
				memset(logBuffer,0,sizeof(logBuffer));

				sprintf(logBuffer,"%s :Server Side :Received packet %s\n",c_timestring,buf);
				LOG(logfile,logBuffer);	
			}
			bill=fopen(buf, "a+");
			if(strcmp(buf,"^C")!=0)
			{
			sem_wait(&sem1);
			totalwaittimeinsec = timeit(cli.start,start);
			*totalwaittime = *totalwaittime + totalwaittimeinsec;
			sem_post(&sem1);
			}
			
			if (DEBUG)
			{
				printf("\nprinting to file will start\n");
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"\n%s printing to file %s will start\n",c_timestring,buf);
				LOG(logfile,logBuffer);	
			}
			if(strcmp(buf,"^C")!=0)
			{
			fprintf(bill,"\n%50s\n",buf);

			fprintf(bill,"\n***************************************************************************************************************\n");

			fprintf(bill,"%-2s %-6s %25s %20s\n","sl#","cust#","Amt.bill","Amt.Due");

			fprintf(bill,"-----------------------------------------------------------------------------------------------------------------\n");
			if (DEBUG)
			{
				printf("\n Before for loop of bill generation\n");
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"\n%s :Before for loop of bill generation\n",c_timestring);
				LOG(logfile,logBuffer);	
			}
			
			for(count = 0;count<10000000;++count)
			{
			   fprintf(bill,"%-2d %-6lu %20lu %20lu\n",count,getrandom(1000000000,9999999999),getrandom	(1000,9999),getrandom(1000,9999));
			}
			}
			if(strcmp(buf,"^C")!=0)
			{
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
			sem_wait(&sem);
			timedelayInSec = timeit(cli.start,end);
			temptime = *totaltime;
			temptotaltime = temptime+timedelayInSec;
			*totaltime = 0;
			*totaltime = temptotaltime;
			sem_post(&sem);
			}
			
			if (DEBUG)
			{
				printf("\n After for loop of bill generation\n");
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
				printf("\n total time till now is %lf seconds ,request = %s",*totaltime,buf);
				sprintf(logBuffer,"\n%s :After for loop of bill generation\n",c_timestring);
				LOG(logfile,logBuffer);	
			}
			fclose(bill);
			if (DEBUG)
			{
				printf("\n Closed the %s region bill file\n",buf);
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"\n%s :Closed the %s region bill file\n",c_timestring,buf);
				LOG(logfile,logBuffer);	
				c_timestring = timestring();
				printf("\n%s region bill generation took %lf seconds to generate the bill",buf,timedelayInSec);
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"\n%s :%s region bill generation took %lf seconds to generate the bill\n",c_timestring,buf,timedelayInSec);
				LOG(logfile,logBuffer);	
			}
			//memset(timestr,0,sizeof(timestr));
			//sprintf(timestr,"%lf\n",timedelayInSec);
			c_timestring = timestring();
			
			memset(logBuffer,0,sizeof(logBuffer));
			sprintf(logBuffer,"\n%s :total time till now is %lf seconds ,request = %s\n",c_timestring,*totaltime,buf);
			LOG(logfile,logBuffer);	
			/*bytesSent = sendto(s,timestr, BUFLEN, 0, &si_other, slen);
			if( bytesSent == -1)
			diep("sendto() error in server side while sending the time of receipt");*/
		} 
		if(strcmp(buf,"^C")==0)
		{
			fclose(logfile);
			break;
		}

	
	}//while end

	if(strcmp(buf,"^C")==0)
	{
		close(s);
		//killallforkedchild();
		//raise(SIGINT);
		
		
	}
}
	
void killallforkedchild()
{	
	int     i;
	char *c_timestring;
	char logBuffer [5120];
	/* terminate all children */
	
	for (i = 0; i < NCHILDREN; i++)
	kill(pids[i], SIGTERM);
	while (wait(NULL) > 0) ;    /* wait for all children */
	
	exit(0);
}


pid_t child_make(int i)
{	
	pid_t   pid;
	void    child_main(int);
	 int pshared =1;
	int pshared1= 1;
	unsigned int value = 1;
	unsigned int value1 = 1;
	 /* Initialize Shared Memory */
	key = ftok("/tmp",'R');
	shmid = shmget(key, (size_t)sizeof(double), 0644 | IPC_CREAT);
	key1 = ftok("/mnt",'R');
    	shmid1 = shmget(key1, (size_t)sizeof(double), 0644 | IPC_CREAT);
	/* Attach to Shared Memory */			
	totaltime = shmat(shmid, (void *)0, 0);
	if(totaltime == (int *)(-1))
    	perror("\nshmat totaltime");
	totalwaittime = shmat(shmid1, (void *)0, 0);
	if(totalwaittime == (int *)(-1))
    	perror("\nshmat totalwait time");
	 
  /* Write initial value to shared memory */
  *totaltime = 0.0;
  *totalwaittime = 0.0;

  /* Initialize Semaphore */
  if((sem_init(&sem, pshared, value)) == 1)
    {
      perror("\nError initializing semaphore for total time");
      exit(1);
    }
    if((sem_init(&sem1, pshared1, value1)) == 1)
    {
      perror("\nError initializing semaphore for total wait time");
      exit(1);
    }
	if ( (pid = fork()) > 0)
	return (pid);            /* parent */
	child_main(i);     /* never returns */
	return 0;
}

void child_main(int i)
{	
	printf("child %ld starting\n", (long) getpid());
	for(;;)
	{
		generatebill(i);
	}
}

int main(int argc, char **argv)
{	
	int i;
	void    sig_int(int);
	char logBuffer [5120];
	char *c_timestring;
	int status;
		
	pids = calloc(NCHILDREN, sizeof(pid_t));
	int s;
	
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	diep("socket");
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, &si_me, sizeof(si_me))==-1)
	diep("bind");

	for (i = 0; i < NCHILDREN; i++)
	{
		pids[i] = child_make(s); /* parent returns */
	}
	signal(SIGINT, sig_int);
	
	for ( ; ; )
	{
		pause();
	}
	
	return 0;

}
void sig_int(int signo)
{
	//int pid,status,serrno;   
 	int     i;
	char logBuffer [5120];
	char *c_timestring;
	printf("\n Signal CTR+C caught");
	printf("\n inside sig_int and before wait(NULL)");
         /* terminate all children */
	for (i = 0; i < NCHILDREN; i++)
    	  kill(pids[i], SIGTERM);
	while (wait(NULL) > 0) ;    /* wait for all children */
	 printf("\n inside sig_int and after wait(NULL)");
	
	avgtimelog = fopen("averagetimelog.log","a+");
	avgtimedelay = *totaltime/10;
	memset(logBuffer,0,sizeof(logBuffer));
	printf("\n average time delay = %lf sec",avgtimedelay);
	c_timestring = timestring();
	sprintf(logBuffer,"\n%s :average time delay = %lf sec\n",c_timestring,avgtimedelay);
	LOG(avgtimelog,logBuffer);
	fclose(avgtimelog);

	avgwaittimelog = fopen("averagewaittimelog.log","a+");
	avgwaittime = *totalwaittime/10;
	memset(logBuffer,0,sizeof(logBuffer));
	printf("\n average waiting time in queue = %lf sec",avgwaittime);
	fflush(stdout);
	c_timestring = timestring();
	sprintf(logBuffer,"\n%s :average waiting time in queue = %lf sec\n",c_timestring,avgwaittime);
	LOG(avgwaittimelog,logBuffer);
	fclose(avgwaittimelog);
	exit(0);
   
	
}

