/*-
 /* Anshuman Biswal
 * All rights reserved.
 * This is the client program using UDP for prefokedserver usecase *    
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

     
#define BUFLEN 1000 
#define NPACK 10
#define PORT 9930
#define DEBUG 1
#define LOG(lf, msg) fprintf(lf, "user: %d: %s\n", getuid(), (msg))

double totaltimedelay;
void diep(char *s)
{
	perror(s);
	exit(1);
}
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
   return number;
}

void generateClient(int clientnum)
{
	struct sockaddr_in si_other;
	
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
	
	if(clientnum == 71)
		sprintf(code,"%s","^C");
	else
		sprintf(code,"%d",clientnum);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	while(1)
	{
		log = fopen("udp_assignment_client.log", "a+");
		bytesSent = sendto(s, code, BUFLEN, 0, &si_other, slen);
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
		if(clientnum == 71)
		break;
		/* Receive endtime for each request*/
		memset(timestr,0,sizeof(timestr));
		if (recvfrom(s, timestr,BUFLEN, 0, &si_other, &slen) == -1)
		{
			diep("recvfrom() in client side");
		}else
		{
			timedelayInSec = strtod(timestr,NULL);
			totaltimedelay=totaltimedelay+timedelayInSec;
			if(DEBUG)
			{
				printf("\n Client Side :Received time for request number %s as %s seconds",code,timestr);
				memset(logBuffer,0,sizeof(logBuffer));	
				c_timestring = timestring();
				sprintf(logBuffer, "%s :Client Side :Received time for request number %s as %s seconds\n",c_timestring,code,timestr);
				LOG(log,logBuffer);
				printf("\n Client Side :after converting time string to double value the value is %lf",timedelayInSec);
				memset(logBuffer,0,sizeof(logBuffer));	
				c_timestring = timestring();
				sprintf(logBuffer, "%s :Client Side :after converting time string to double value the value is %lf",c_timestring,code,timestr);
				LOG(log,logBuffer);
				printf("\n Client Side :total time delay till now is %lf seconds",totaltimedelay);
				memset(logBuffer,0,sizeof(logBuffer));	
				c_timestring = timestring();
				sprintf(logBuffer, "\n %s :Client Side :total time delay till now is %lf seconds",c_timestring,totaltimedelay);
				LOG(log,logBuffer);
			}
			break;
		}
		
	}
	
	if(clientnum == 71)
	{
		averagetimedelaylog = fopen("averagetimedelay.log","a+");
		averagetimedelayinsec = totaltimedelay/70;
		printf("\n Client Side :average time delay of all client requests is %lf seconds",averagetimedelayinsec);
		memset(logBuffer,0,sizeof(logBuffer));
		c_timestring = timestring();
		sprintf(logBuffer, "\n %s :Client Side :average time delay of all client requests is %lf seconds",c_timestring,averagetimedelayinsec);
		LOG(averagetimedelaylog,logBuffer);
		fclose(log);
		fclose(averagetimedelaylog);
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
/* Intializes random number generator for the seed */
	srand((unsigned) time(&t));
	for(clientnum = 1;clientnum<=71;clientnum++)
	{
	 	printf("\n client number %d sending to generate bill",	clientnum);   
		generateClient(clientnum);
		if(clientnum != 70)
		{
			//sleeptime = getrandom(0,52);//for 0.1 system utilization for 70 requests with 7 preforked child server
			//printf("\n sleep time = %lu",sleeptime);
			//usleep(sleeptime);
		}
	}
	return 0;
}
