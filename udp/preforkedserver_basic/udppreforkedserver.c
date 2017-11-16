/* Anshuman Biswal
* All rights reserved.
* This is the server program using UDP for preforked server usecase*    
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

#define BUFLEN 1000
#define NPACK 1000
#define PORT 9930
#define NCHILDREN 7
#define LOG(lf, msg) fprintf(lf, "\n child:%ld user:%d: %s\n",(long)getpid(), getuid(), (msg))

#define DEBUG 1
void diep(char *s)
{
	perror(s);
	exit(1);
}
FILE *logfile; 
static pid_t *pids;
double timedelayInSec;
double avgtimedelay;
struct timespec start,end;
int     listenfd, nthreads;
struct sockaddr_in si_me, si_other;

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

double diff(struct timespec start, struct timespec end)
{	
	double starttimeinMS,endTimeinMS,diffTimeinMS;
	if (start.tv_nsec < end.tv_nsec)
	{
		int adj = (end.tv_nsec - start.tv_nsec) / (1000000000) + 1;
		end.tv_nsec -= (1000000000) * adj;
		end.tv_sec += adj;
	}
	if (start.tv_nsec - end.tv_nsec > (1000000000))
	{
		int adj = (start.tv_nsec - end.tv_nsec) / (1000000000);
		end.tv_nsec += (1000000000) * adj;
		end.tv_sec -= adj;
	}
	starttimeinMS = (start.tv_sec)+(start.tv_nsec/1000000000.0);
	endTimeinMS = (end.tv_sec)+( end.tv_nsec/1000000000.0);
	diffTimeinMS = (double)(endTimeinMS-starttimeinMS);
	return diffTimeinMS;
}

void generatebill()
{	time_t t;
	int count=0;
	struct timespec end;
	size_t bytesSent;
	char buf[BUFLEN];
	char logBuffer [5120];
	char timestr[BUFLEN];
	char *c_timestring;
	void killallforkedchild();
	FILE *bill;
	logfile=fopen("udp_assignment_server.log", "a+");
	int s, slen=sizeof(si_other);
	
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	diep("socket");
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, &si_me, sizeof(si_me))==-1)
	diep("bind");

	while(1) 
	{
		memset(buf,0,sizeof(buf));
		if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
		{
			diep("recvfrom()");
		}else
		{
			clock_gettime(CLOCK_MONOTONIC_RAW,&end);
			c_timestring = timestring();	
			if(DEBUG)
			{
				printf ("\n messgae receivedv = %s ", buf);
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"%s :Server Side :Received packet %s\n",c_timestring,buf);
				LOG(logfile,logBuffer);	
			}
			if(strcmp(buf,"^C")==0)
			break;
			bill=fopen(buf, "a+");
			/* Intializes random number generator */
			srand((unsigned) time(&t));
			
			if (DEBUG)
			{
				printf("\nprinting to file will start\n");
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
				sprintf(logBuffer,"\n%s printing to file %s will start\n",c_timestring,buf);
				LOG(logfile,logBuffer);	
			}
			clock_gettime(CLOCK_MONOTONIC_RAW,&start);
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
				fprintf(bill,"%-2d %-6lu %20lu %20lu\n",count,getrandom(1000000000,9999999999),getrandom(1000,9999),getrandom(1000,9999));
			}
			
			clock_gettime(CLOCK_MONOTONIC_RAW,&end);
			timedelayInSec = diff(start,end);
			if (DEBUG)
			{
				printf("\n After for loop of bill generation\n");
				c_timestring = timestring();
				memset(logBuffer,0,sizeof(logBuffer));
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
			memset(timestr,0,sizeof(timestr));
			sprintf(timestr,"%lf\n",timedelayInSec);
			bytesSent = sendto(s,timestr, BUFLEN, 0, &si_other, slen);
			if( bytesSent == -1)
			diep("sendto() error in server side while sending the time of receipt");
		} 
	}
	close(s);
	killallforkedchild();
	
	
}

void killallforkedchild()
{	
	int     i;
	char *c_timestring;
	char logBuffer [5120];
	/* terminate all children */
	if (DEBUG)
	{
		printf("\n terminate all children\n");
		c_timestring = timestring();
		memset(logBuffer,0,sizeof(logBuffer));
		sprintf(logBuffer,"\n%s :terminate all pre forked children\n",c_timestring);
		LOG(logfile,logBuffer);	
	}
	for (i = 0; i < NCHILDREN; i++)
	kill(pids[i], SIGTERM);
	while (wait(NULL) > 0) ;    /* wait for all children */
	fclose(logfile);
	exit(0);
}


pid_t child_make(int i)
{	
	pid_t   pid;
	void    child_main(int);
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
		generatebill();
		//close(listenfd);
	}
}

int main(int argc, char **argv)
{	
	int i;
	void    sig_int(int);	
	pids = calloc(NCHILDREN, sizeof(pid_t));
	for (i = 0; i < NCHILDREN; i++)
	{
		pids[i] = child_make(i); /* parent returns */
	}
	signal(SIGINT, sig_int);
	for ( ; ; )
	pause();
	return 0;

}
void sig_int(int signo)
{
    int     i;

         /* terminate all children */
     for (i = 0; i < NCHILDREN; i++)
         kill(pids[i], SIGTERM);
     while (wait(NULL) > 0) ;    /* wait for all children */

     exit(0);
}

