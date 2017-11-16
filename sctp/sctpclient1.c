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
#include <errno.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */
#define PORT 4500
int debug=0;


int generateClient(int clientnum)
{
	int sock_fd, sz, len, msg_flags;
	int idleTime = 2,reqCount=0;
	struct sockaddr_in sin[1], serv_addr;
	struct sctp_event_subscribe event;
	char buf[256],tempbuf[256]; 	
	struct sctp_sndrcvinfo sri;
	/*if (argc < 3) {
		printf ("\nUsage: <%s> <remote-ip> <port> \n\n", argv[0]);
		return -1;
	}*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (PORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_fd = socket (AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	if (sock_fd == -1) {
		printf ("\nUnable to create socket \n");
		return -1;
	}
	if (connect (sock_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf ("\nUnable to connect to remote server [%s] !! \n\n",inet_ntoa(serv_addr.sin_addr));
		return -1;
	} else {
		printf ("\nConnected to [%s]\n", inet_ntoa(serv_addr.sin_addr));
	}
	memset (&event, 0, sizeof(event));
	if (setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS,&event, sizeof(event)) != 0) {
		perror("setevent failed");
		exit(1);
	}
	memset (buf, 0, sizeof(buf));
	len = sizeof(serv_addr);

       printf ("Enter region name: ");
       while (fgets(buf, 256, stdin)) {

	    sri.sinfo_stream = strtol(buf+1, NULL, 0);
	    sz = strlen(buf);
	    len = sizeof(serv_addr);
	    sz = sctp_sendmsg (sock_fd, buf, sz, &serv_addr, len, 0, 0,sri.sinfo_stream, 0, 0);
	    printf ("sctp_sendmsg:[%d,e:%d]\n", sz,errno);
	  
	   sz = sctp_recvmsg (sock_fd, tempbuf, sizeof(tempbuf),&serv_addr, &len, &sri, &msg_flags);
	    printf ("sctp_recvmsg:[%d] ", sz);
	    if (sz <= 0)
	    break;
	   
	    printf ("<-- %s on str %d\n", tempbuf, sri.sinfo_stream);
	     if(strcmp(buf,tempbuf)==0)
	   {
	    
	    memset (buf, 0, sizeof(buf));
	    memset (tempbuf, 0, sizeof(tempbuf));
	    break;
	  }
	      
	}    
            
	printf ("\nClient # %d Over !!",clientnum);
	close (sock_fd);
}
int main(int argc, char **argv)
{
	
	int clientnum =0;
	for(clientnum = 1;clientnum<16;clientnum++)
	generateClient(clientnum);
	
	return 0;
}
