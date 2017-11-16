/*-
 * All rights reserved.
 * Name: Anshuman Biswal
 */
 #include    "unp.h"

 #define MAXN    16384     /* max # bytes to request from server */

 int main(int argc, char **argv)
  {
      int     i, j, fd, nchildren, nloops, nbytes;
      pid_t   pid;
      ssize_t n;
      char    request[MAXN], reply[MAXN];

     if (argc != 6)
	{
         printf("usage: %s <hostname or IPaddr> <port> <#children> "
                   "<#loops/child> <#bytes/request>",argv[0]);
	exit(1);
	}

 nchildren = atoi(argv[3]);
 nloops = atoi(argv[4]);
 nbytes = atoi(argv[5]);
 snprintf(request, sizeof(request), "%d\n", nbytes); /* newline at end */

 for (i = 0; i < nchildren; i++) {
        if ( (pid = fork()) == 0) { /* child */
             for (j = 0; j < nloops; j++) {
                 fd = connect(argv[1], argv[2]);

                 write(fd, request, strlen(request));

                 if ( (n = read(fd, reply, nbytes)) != nbytes)
		{	
                     printf("\nserver returned %d bytes", n);
		     exit(0);
		}	

                 Close(fd);       /* TIME_WAIT on client, not server */
             }
             printf("child %d done\n", i);
             exit(0);
         }
         /* parent loops around to fork() again */
     }

     while (wait(NULL) > 0);     /* now parent waits for all children */
        
     exit(0);
}

