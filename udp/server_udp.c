 /* 
 * This is the client program using UDP and it was written as part of assignment for module 3 for MSC Engg course in the *    
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
     
#define BUFLEN 1000
#define NPACK 1000
#define PORT 9930
#define LOG(lf, msg) fprintf(lf, "user: %d: %s\n", getuid(), (msg))

void diep(char *s)
{
	perror(s);
	exit(1);
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
    
int main(void)
{
	struct sockaddr_in si_me, si_other;
	struct timespec end;
	size_t bytesSent;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];
	char logBuffer [5120];
	char *c_timestring;
	FILE *log = fopen("udp_assignment_server.log", "a+");
    
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
		if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
		{
			diep("recvfrom()");
		}
		else
		{
			//clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
			clock_gettime(CLOCK_MONOTONIC_RAW,&end);
			c_timestring = timestring();		
			sprintf(logBuffer,"%s :Server Side :Received packet %s\n",c_timestring,buf);
			LOG(log,logBuffer);	
			//Sending the time of receipt of packets
			bytesSent = sendto(s, &end, (int)sizeof(end), 0, &si_other, slen);
			if( bytesSent == -1)
				diep("sendto() error in server side while sending the time of receipt");
			else
			{
				c_timestring = timestring();			
				//printf("Server Side Side :Sending time of receipt of packet as %d seconds and %ld nanaoseconds\n",end.tv_sec,end.tv_nsec);
			}
			//printf("Received packet %s\n", buf);
		}
		if(strcmp(buf,"^C")==0)
		break;
		
	}
   
close(s);
fclose(log);
return 0;
}
