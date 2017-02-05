/*
        demo-udp-03: udp-send: a simple udp client
	send udp messages
	This sends a sequence of messages (the # of messages is defined in MSGS)
	The messages are sent to a port defined in SERVICE_PORT 

        usage:  udp-send

        Paul Krzyzanowski
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "port.h"
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/tcp.h>
// #define _GNU_SOURCE
#include <sched.h>
#include <limits.h>

void setcpuaffinity() {
  /* unsigned long mask = 2;  */
  /* unsigned int len = sizeof(mask); */
  /* if (sched_setaffinity(0, len, &mask) < 0) { */
  /*   perror("sched_setaffinity"); */
  /* } */
  cpu_set_t  mask;
  CPU_ZERO(&mask);
  CPU_SET(2, &mask);
  if( sched_setaffinity(0, sizeof(mask), &mask) < 0) {
     perror("sched_setaffinity");
  }
}


// #define BUFLEN 2048
// #define MSGS 1	/* number of messages to send */
// #define MAX_PACKET_SIZE 6500

int main(int argc, char *argv[])
{
	struct sockaddr_in myaddr, remaddr;
	int fd, i, slen=sizeof(remaddr);
	int size = atoi(argv[1]);
	char buf[size];	/* message buffer */
	int recvlen;		/* # bytes in acknowledgement message */
	char *server = "127.0.0.1";	/* change this to use a different server */

	setcpuaffinity();

	FILE *fp = fopen("../random2", "r");
	fread(&buf, size, sizeof(char), fp);
	buf[size] = 0;

	int MSGS = atoi(argv[2]);
    
	/* create a socket */

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}       

	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(SERVICE_PORT);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	double minValue = INT_MAX, avgValue = 0.0;
	/* now let's send the messages */
	int j;
	for (i=0; i < MSGS; i++) {
	  struct timespec requestStart, requestEnd;
	  clock_gettime(CLOCK_MONOTONIC, &requestStart);
	  // printf("Sending packet %d to %s port %d\n", i, server, SERVICE_PORT);
	
	  // sprintf(buf, "This is packet %d", i);

	  int count = 0;
	  int j = size;
	  int iter = 0;
	  for(; j > 0; j-= MAX_PACKET_SIZE) {
	    if (sendto(fd, &buf[iter], j < MAX_PACKET_SIZE ? j : MAX_PACKET_SIZE, 0, (struct sockaddr *)&remaddr, slen)==-1) {
	      perror("sendto");
	      exit(1);
	    }
	    iter+=MAX_PACKET_SIZE;
	    // printf("sending iter=%d\n", iter);
	  }
	  // printf("sending done\n");
	  /* now receive an acknowledgement from the server */
	  int k = 0;
	  while((recvlen = recvfrom(fd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &slen)) > 0) {
	    k+=recvlen;
	    // printf("recv debug: %d\n", recvlen);
	    if(k >= size) break;
	  }

	  // End measurement
	  clock_gettime(CLOCK_MONOTONIC, &requestEnd);
          // Calculate time it took
	  double accum = ( requestEnd.tv_sec - requestStart.tv_sec )*1000000000.0
	    + ( requestEnd.tv_nsec - requestStart.tv_nsec );
	  
	  if(accum < minValue) minValue = accum;
	  avgValue += accum;


	  if(recvlen < 0) {
	    printf("Error: Got -1 from recvfrom\n");
	    exit(5);
	  }
	  if(size != k) {
	    printf("Error: recvfrom mismatch\n");
	    exit(6);
	  }
		
	}

	printf("Min latency\t%lf\tAvglatency\t%f\n", minValue, avgValue/MSGS);
	close(fd);
	return 0;
}
