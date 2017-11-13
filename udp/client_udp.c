/*-
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
#include "vector.h"
     
#define BUFLEN 1000 
#define NPACK 10
#define PORT 9930
#define LOG(lf, msg) fprintf(lf, "user: %d: %s\n", getuid(), (msg))
#define LOGTIMEDELAY(lf, msg) fprintf(lf, "%s\n",(msg))
const int NANO_SECONDS_IN_SEC = 1000000000;

struct timespec start,totalStart,averageTime,averageEndTime;

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
 vector *readFile(char *fileName,int bytesize)
{
  FILE *file,*logFile;
  vector *dataHolder = vector_create(1000*sizeof (char*),0);
  char logBuffer [5120];
  memset(&totalStart, 0, sizeof(totalStart));
  char *code = malloc(10000 * sizeof(char));
  int averageSeconds;
  long averageNanoseconds;
  file = fopen(fileName, "r");
  logFile = fopen("udp_assignment_client.log", "a+");
  int ch;
  int i = 0;/* number of characters seen */
 //printf("\n starting reading file");
  while(1)
  {
	code[i++] = (char)fgetc(file);
	code[i] = '\0';
	vector_insert(dataHolder,code,i-1);
	if((code[i-1]==EOF)||(i-1==bytesize))
	{
		break;
	}
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);
	clock_gettime(CLOCK_MONOTONIC_RAW,&start);
	totalStart.tv_sec+= start.tv_sec;
	totalStart.tv_nsec+= start.tv_nsec;
	usleep(5000);
     	
  } 
  
  averageTime.tv_sec = totalStart.tv_sec/bytesize;
  averageTime.tv_nsec = totalStart.tv_nsec/bytesize;
  sprintf(logBuffer, "Client Side :Average time of bytes of a total of  %d size packet is %d seconds and %ld nanoseconds\n",bytesize,averageTime.tv_sec,averageTime.tv_nsec);
  LOG(logFile,logBuffer);
  fclose(file);
  fclose(logFile);
  return dataHolder;
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
	starttimeinMS = (start.tv_sec * 1000)+(start.tv_nsec/1000000.0);
	endTimeinMS = (end.tv_sec*1000)+( end.tv_nsec/1000000.0);
	diffTimeinMS = (double)(endTimeinMS-starttimeinMS);
		
	return diffTimeinMS;

	
}	

#define SRV_IP "127.0.0.1"

int main(void)
{
	struct sockaddr_in si_other;
	
	int s, i=0;
	int slen=sizeof(si_other);
	int ch;
	int byteSize = 1;
	size_t bytesSent;
	char *code = malloc(10000 * sizeof(char));
	char *data;
	char logBuffer [5120];
	char *c_timestring;
	int randomNumber;
	struct timespec end_Time,totalEnd_Time,time_delay;
	double timedelayInMilliSec;
	int choice;
	FILE *file,*log,*timedelayLogSingleTransmission,*timedelayLogbatchTransmission;
	struct timespec time1, time2;
	
	
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");
    
	memset((char *) &si_other, 0, sizeof(si_other));

	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	printf("\n enter choice:");
	printf("\n 1: to send byte by byte data as it is");
	printf("\n 2: to send data batch wise");
	printf("\n 3: to exit\n");
	scanf("%d", &choice);
	while(1)
	{
		switch(choice)
		{
			case 1:
			{
								
				file = fopen("char.txt", "r");
				log = fopen("udp_assignment_client.log", "a+");
				timedelayLogSingleTransmission = fopen("timeDelay_Log_singlebyteTrans.log","a+");
  				while(1)
  				{
    				code[i++] = (char)fgetc(file);
					//printf("\n byte read code is %s",code);
					//clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);
					clock_gettime(CLOCK_MONOTONIC_RAW,&start);
					bytesSent = sendto(s, code, BUFLEN, 0, &si_other, slen);
					if( bytesSent == -1)
					diep("sendto()");
					else
					{
						c_timestring = timestring();			
						sprintf(logBuffer, "%s :Client Side :Sending packet %s of size %d\n",c_timestring,code,strlen(code));
						LOG(log,logBuffer);
					}
					/* Receive endtime for each byte*/
					if (recvfrom(s, (struct timespec *)&end_Time, (int)sizeof(end_Time), 0, &si_other, &slen) == -1)
					{
						diep("recvfrom() in client side");
					}else
					{
											
						sprintf(logBuffer, "%s :Client Side :Received time of packets of size %d received in server side as %d seconds and %ld nanoseconds\n",c_timestring,byteSize,end_Time.tv_sec,end_Time.tv_nsec);
						LOG(log,logBuffer);
						//end_Time.tv_sec+=start.tv_sec;
						//end_Time.tv_nsec+=start.tv_nsec;
						timedelayInMilliSec = diff(start,end_Time);
						//timedelayInMilliSec = (time_delay.tv_sec * 1000)+(time_delay.tv_nsec/1000000.0);
						sprintf(logBuffer, "%s :Client Side :Time delay (single byte transmission)for %d packet size is %lf milliseconds	\n",c_timestring,strlen(code),timedelayInMilliSec);
						//printf("Client Side :Time delay  for %d packet size is %lf milliseconds\n",byteSize,timedelayInMilliSec);
						LOG(timedelayLogSingleTransmission,logBuffer);
					}
					if((code[i-1]==EOF)||(i==1000))
					{
						fclose(file);	
						break;	
						
					}
					usleep(5000);
				}
				
				
			}break;
			case 2:
			{
								
				//printf("\n sending packet batch wise starting with 1 byte size");
				log = fopen("udp_assignment_client.log", "a+");
				timedelayLogbatchTransmission = fopen("timeDelay_BatchTransmissionLog2.log","a+");
				vector *dataHolder = vector_create(sizeof (char*),0) ;
				while(1)
				{				
					dataHolder = readFile("char.txt",byteSize);
					memset(&totalEnd_Time, 0, sizeof(totalEnd_Time));
					for(i=0;i<byteSize;i++)
					{
						data = (char*)vector_at(dataHolder,i);
						bytesSent = sendto(s, data, BUFLEN, 0, &si_other, slen);
						if( bytesSent == -1)
							diep("sendto()");
						else
						{
							c_timestring = timestring();			
							sprintf(logBuffer, "%s :Client Side :Sending packet %s of size %d\n",c_timestring,data,strlen(data));
							LOG(log,logBuffer);
						}
						
						/* Receive endtime for each byte*/
						if (recvfrom(s, (struct timespec *)&end_Time, (int)sizeof(end_Time), 0, &si_other, &slen) == -1)
						{
							diep("recvfrom() in client side");
						}else
						{
							totalEnd_Time.tv_sec+= end_Time.tv_sec;
							totalEnd_Time.tv_nsec+= end_Time.tv_nsec;
							sprintf(logBuffer, "%s :Client Side :Received time of packets of size %d received in server side as %d seconds and %ld nanoseconds\n",c_timestring,byteSize,end_Time.tv_sec,end_Time.tv_nsec);
							LOG(log,logBuffer);

						}
						
					}
					averageEndTime.tv_sec = totalEnd_Time.tv_sec/byteSize;
					averageEndTime.tv_nsec = totalEnd_Time.tv_nsec/byteSize;
					//averageEndTime.tv_sec+=averageTime.tv_sec;//Since the byte took the averageTime to get generated
					//averageEndTime.tv_nsec+=averageTime.tv_nsec;
					sprintf(logBuffer, "%s :Average End Time of packet of size %d received in server side is as %d seconds and %ld nanoseconds\n",c_timestring,byteSize,averageEndTime.tv_sec,averageEndTime.tv_nsec);
					LOG(log,logBuffer);
					timedelayInMilliSec = diff(averageTime,averageEndTime);
					//printf("\n%s :Client Side :Time delay for %d pacekt size is %d seconds and %ld nanoseconds\n",c_timestring,byteSize,time_delay.tv_sec,time_delay.tv_nsec);
					/*converting seconds and nano seconds to milli seconds*/
					//timedelayInMilliSec = (time_delay.tv_sec * 1000)+(time_delay.tv_nsec/1000000.0);
					sprintf(logBuffer, "%s :Client Side :Time delay (batch wise transmission)for %d pacekt size is %lf milliseconds\n",c_timestring,byteSize,timedelayInMilliSec);
					//sprintf(logBuffer, "%lf milliseconds",timedelayInMilliSec);
					//printf("Client Side :Time delay  for %d pacekt size is %lf milliseconds\n",byteSize,timedelayInMilliSec);
					LOG(timedelayLogbatchTransmission,logBuffer);
					//LOGTIMEDELAY(timedelayLogbatchTransmission,logBuffer);
						byteSize+=39;
					vector_clear(dataHolder);

					//	printf("\n now the byte size will be %d",byteSize);
					if(byteSize > 1001)
					{
						break;
					}
					
				}
				
				fclose(log);
				fclose(timedelayLogbatchTransmission);
				
			}break;
			case 3:
			{
				log = fopen("udp_assignment_client.log", "a+");
				data = "^C";				
				bytesSent = sendto(s, data, BUFLEN, 0, &si_other, slen);
				if( bytesSent == -1)
				 	diep("sendto()");
				else
				{
					c_timestring = timestring();			
					sprintf(logBuffer, "%s :Client Side :Sending packet %s of size %d\n",c_timestring,data,strlen(data));
					LOG(log,logBuffer);
				}
			fclose(log);
			exit(0);
			}break;
			default:
			{
				printf("\n enter choice:");

				printf("\n 1: to send byte by byte data as it is");

				printf("\n 2: to send data batch wise");

				printf("\n 3: to exit\n");

				scanf("%d", &choice);

				
			}break;
		}
		printf("\n enter choice:");

		printf("\n 1: to send byte by byte data as it is");

		printf("\n 2: to send data batch wise");

		printf("\n 3: to exit\n");

		scanf("%d", &choice);	
		
	}

	 

return 0;
}
