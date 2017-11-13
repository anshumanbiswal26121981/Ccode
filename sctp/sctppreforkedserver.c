/*-
 /* 
 * This is the Server program using SCTP  *    
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
#include <signal.h>
#include <errno.h>
#define BUFLEN 100
#define LOG(lf, region) fprintf(lf, "%s: %d: %s\n", getuid(), (msg))
int debug=0;
static int nchildren;
static pid_t *pids;


struct timespec start,end;

int     listenfd, nthreads;
 socklen_t addrlen;

unsigned long getrandom(int start,int end)
{
  unsigned long number;
   number= rand() % (end - start + 1) + start;
 // printf("%lu\n",number);
  return number;
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
void generatebill(char* regionname)
{
	time_t t;
	FILE* timelog=fopen("timelog.txt", "a+");
	FILE* billog =fopen("bill_all_region.txt", "a+");
	double timedelayInMilliSec;
	
	int i;
	
	
	/* Intializes random number generator */
	   srand((unsigned) time(&t));
	clock_gettime(CLOCK_MONOTONIC_RAW,&start);
	printf("\nprinting to file will start\n");
	fprintf(billog,"\n%50s\n",regionname);
	fprintf(billog,"\n***************************************************************************************************************\n");
	printf("\nprinting to file :header\n");
	fprintf(billog,"%-2s %-6s %25s %20s\n","sl#","cust#","Amt.bill","Amt.Due");
	printf("\nprinting to file :sub header\n");
	
	fprintf(billog,"-----------------------------------------------------------------------------------------------------------------\n");
	printf("\nprinting to file :before for loop\n");	
	for(i = 0;i<10000000;++i)
	{
		fprintf(billog,"%-2d %-6lu %20lu %20lu\n",i,getrandom(1000000000,9999999999),getrandom(1000,9999),getrandom(1000,9999));
		
	}
	printf("\nprinting to file :afterfor loop\n");

	clock_gettime(CLOCK_MONOTONIC_RAW,&end);
	timedelayInMilliSec = diff(start,end);
	printf("%s took %lf time",regionname,timedelayInMilliSec);
	
	fprintf(timelog,"\nbill generation for %s region took %lf seconds",regionname,timedelayInMilliSec);
	fclose(billog);
	fclose(timelog);
} 



static void handle_event(void *buf)
{
	struct sctp_assoc_change *sac;
	struct sctp_send_failed *ssf;
	struct sctp_paddr_change *spc;
	struct sctp_remote_error *sre;
	union sctp_notification *snp;
	char addrbuf[INET6_ADDRSTRLEN];
	const char *ap;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	snp = buf;
	switch (snp->sn_header.sn_type) 
	{
		case SCTP_ASSOC_CHANGE:
		{
			sac = &snp->sn_assoc_change;
			printf("^^^ assoc_change: state=%hu, error=%hu, instr=%hu outstr=%hu\n", sac->sac_state, sac->sac_error,sac->sac_inbound_streams, sac->sac_outbound_streams);
		}
			break;
		case SCTP_SEND_FAILED:
		{
			ssf = &snp->sn_send_failed;
			printf("^^^ sendfailed: len=%hu err=%d\n", ssf->ssf_length,ssf->ssf_error);
		}
			break;
		case SCTP_PEER_ADDR_CHANGE:
		{
			spc = &snp->sn_paddr_change;
			if (spc->spc_aaddr.ss_family == AF_INET) 
			{
				sin = (struct sockaddr_in *)&spc->spc_aaddr;
				ap = inet_ntop(AF_INET, &sin->sin_addr,addrbuf, INET6_ADDRSTRLEN);
			} else 
			{
				sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
				ap = inet_ntop(AF_INET6, &sin6->sin6_addr,addrbuf, INET6_ADDRSTRLEN);
			}
			printf("^^^ intf_change: %s state=%d, error=%d\n", ap,spc->spc_state, spc->spc_error);
		}
			break;
		case SCTP_REMOTE_ERROR:
		{
			sre = &snp->sn_remote_error;
			printf("^^^ remote_error: err=%hu len=%hu\n",ntohs(sre->sre_error), ntohs(sre->sre_length));
		}
			break;
		case SCTP_SHUTDOWN_EVENT:
		{
			printf("^^^ shutdown event\n");
		}
			break;
		default:
		{
			printf("unknown type: %hu\n", snp->sn_header.sn_type);
		}
			break;
	}
}

int main(int argc, char **argv)
{
	int fd,i;
	int idleTime = 20;
	struct sockaddr_in sin[1], cli_addr;
	struct sctp_event_subscribe event;
	void    sig_int(int);
	
 	if (argc < 3) {
		printf ("\nUsage: <%s> <port> <#children>\n\n", argv[0]);
		return -1;
	}
	nchildren = atoi(argv[argc - 1]);

	if ((fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) == -1) {
		perror("socket");
		exit(1);
	}
	sin->sin_family = AF_INET;
	sin->sin_port = htons(atoi(argv[1]));
	sin->sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *)sin, sizeof (*sin)) == -1) {
		perror("bind");
		exit(1);
	}
	#if 0
	/* Not interested in any events for now */
	memset (&event, 0, sizeof(event));
	#else
	/* Enable all events */
	event.sctp_data_io_event = 1;
	event.sctp_association_event = 1;
	event.sctp_address_event = 1;
	event.sctp_send_failure_event = 1;
	event.sctp_peer_error_event = 1;
	event.sctp_shutdown_event = 1;
	event.sctp_partial_delivery_event = 1;
	event.sctp_adaptation_layer_event = 1;
	#endif
	if (setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &event, sizeof(event)) != 0) {
		perror("setevent failed");
		exit(1);
	}
	/* Set associations to auto-close in 20 seconds of inactivity*/
	/*if (setsockopt(fd, IPPROTO_SCTP, SCTP_AUTOCLOSE,&idleTime, 4) < 0) {
		perror("setsockopt SCTP_AUTOCLOSE");
		exit(1);
	}*/
	/* Allow new associations to be accepted */

	if (listen(fd, 1) < 0) {
		perror("listen");
		exit(1);
	}
	
	pids = calloc(nchildren, sizeof(pid_t));
	for (i = 0; i < nchildren; i++)
	{
        	pids[i] = child_make(i, fd, sizeof (*sin)); /* parent returns */
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
     for (i = 0; i < nchildren; i++)
         kill(pids[i], SIGTERM);
     while (wait(NULL) > 0) ;    /* wait for all children */

     exit(0);
}


pid_t child_make(int i, int listenfd, int addrlen)
{
    pid_t   pid;
   void    child_main(int, int, int);

   if ( (pid = fork()) > 0)
        return (pid);            /* parent */

    child_main(i, listenfd, addrlen);     /* never returns */
     return 0;
}

void child_main(int i, int listenfd, int addrlen)
{
  	char readbuf[256]={0};
	int sz,len,msg_flags;
	struct sockaddr_in  cli_addr;
	struct sctp_sndrcvinfo sri;
	
	memset (&sri, 0, sizeof(sri));
	printf ("{one-to-many}: Waiting for associations ...\n");
	/* Wait for new associations */
	/* Wait for new associations */
	while(1) {
	/* Echo back any and all data */
		memset (readbuf, 0, sizeof(readbuf));
		len = sizeof (struct sockaddr_in);
		sz = sctp_recvmsg (listenfd, readbuf, sizeof(readbuf),&cli_addr, &len, &sri, &msg_flags);
		if (debug)
		printf ("sctp_recvmsg:[%d,e:%d,fl:%X]: ", sz, errno, msg_flags);
		if (sz <= 0)
		break;
		if (msg_flags & MSG_NOTIFICATION) {
		handle_event(readbuf);
		continue;
		}
		printf ("<-- %s on str: %d\n", readbuf, sri.sinfo_stream);
		generatebill(readbuf);
		sz = sctp_sendmsg (listenfd, readbuf, sz, &cli_addr, len,sri.sinfo_ppid, sri.sinfo_flags,sri.sinfo_stream, 0, 0);
		if (debug)
		printf ("sctp_sendmsg:[%d,e:%d]\n", sz, errno);
		
	}
		close(listenfd);
}
