/*-
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
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

     
#define BUFLEN 1000 
#define NPACK 10
#define PORT 9930
#define DEBUG 0
#define LOG(lf, msg) fprintf(lf, "user: %d: %s\n", getuid(), (msg))

double totaltimedelay;
struct timespec starttime;
//struct timeval starttime;
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

char* timestring()
{
# define TIME_SIZE 60

  const struct tm *tm;
  size_t len;
  size_t bytesSent;
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
unsigned long getrandom(int start,int end)
{
  unsigned long number;
   number= rand() % (end - start + 1) + start;
	if (number>end)
		number = end;
   return number;
}

void generateClient(int clientnum)
{
	struct sockaddr_in si_other;
	struct clientinfo cli;
	struct rusage ru;
	int s, i=0;
	int slen=sizeof(si_other);
	int ch;
	int byteSize = 1;
	size_t bytesSent;
	char logBuffer [5120];
	char code[BUFLEN];
	char timestr[BUFLEN];
	char *c_timestring;
	double timedelayInSec=0.0;
	double averagetimedelayinsec=0.0;
	FILE *log,*averagetimedelaylog;
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");
    
	memset((char*) &si_other, 0, sizeof(si_other));
	memset(code,0,sizeof(code));
	memset((char*) &cli, 0, sizeof(cli));
	
		switch(clientnum)
		{
			case 1:{
				  sprintf(code,"%s","delhi");
			       }
			break;
			case 2:
				{
				  sprintf(code,"%s","mumbai");
			       }break;
			case 3:{
				  sprintf(code,"%s","chennai");
			       }break;
			case 4:{
				  sprintf(code,"%s","Pune");
			       }break;
			case 5:{
				  sprintf(code,"%s","Kolkata");
			       }break;
			case 6:{
				  sprintf(code,"%s","Bangalore");
			       }break;
			case 7:{
				  sprintf(code,"%s","Faridabad");
			       }break;
			case 8:{
				  sprintf(code,"%s","Gurgaon");
			       }break;
			case 9:{
				  sprintf(code,"%s","Gaziabad");
			       }break;
			case 10:{
				  sprintf(code,"%s","Goa");
			       }break;
			case 11:{
						sprintf(code,"%s","^C");
				}break;
		}		
	
	
	
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	log = fopen("udp_assignment_client.log", "a+");
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&starttime);
	//getrusage(RUSAGE_SELF,&ru);
	strcpy(cli.name,code);
	//cli.start = ru.ru_utime;
	cli.start=starttime;
	bytesSent = sendto(s, &cli, (int)sizeof(cli), 0, &si_other, slen);
	if( bytesSent == -1)
		diep("sendto()");
	else
	{
		if(DEBUG)
		{
			printf("\n Sending request for client number %s",code);
			c_timestring = timestring();
			memset(logBuffer,0,sizeof(logBuffer));
			sprintf(logBuffer, "%s :Client Side :Sending request for client number %s\n",c_timestring,code);
			LOG(log,logBuffer);
		}
	}
		
	
	if(clientnum == 11)
	{
		
		fclose(log);
		close(s);
		exit(0);
		
	}
	
	close(s);		

}

int main(void)
{
	int clientnum =0;
	unsigned long sleeptime;
	time_t t;
	for(clientnum=1;clientnum<=11;clientnum++){
	generateClient(clientnum);
	}

	return 0;
}
