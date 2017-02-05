/*
        demo-udp-03: udp-recv: a simple udp server
	receive udp messages

        usage:  udp-recv

        Paul Krzyzanowski
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "port.h"
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


// #define BUFSIZE 2048
// #define MAX_PACKET_SIZE 6500

void setcpuaffinity() {
  /* unsigned long mask = 2;  */
  /* unsigned int len = sizeof(mask); */
  /* if (sched_setaffinity(0, len, &mask) < 0) { */
  /*   perror("sched_setaffinity"); */
  /* } */
  cpu_set_t  mask;
  CPU_ZERO(&mask);
  CPU_SET(3, &mask);
  if( sched_setaffinity(0, sizeof(mask), &mask) < 0) {
     perror("sched_setaffinity");
  }
}

int
main(int argc, char **argv)
{
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int fd;				/* our socket */
	int msgcnt = 0;			/* count # of messages we received */
	int size = atoi(argv[1]);
	unsigned char buf[size];	/* receive buffer */

	//Set cpu affinity
	setcpuaffinity();
	int MSGS = atoi(argv[2]);
	
	unsigned char sendbuff[size];
	FILE *fp = fopen("../random2", "r");
	fread(&sendbuff, size, sizeof(char), fp);
	sendbuff[size] = 0;

	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */
	// for (;;) {
	printf("waiting on port %d\n", SERVICE_PORT);
	int i;
	for(i = 0; i < MSGS; i++) {
	  int k = 0;
	  while((recvlen = recvfrom(fd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen)) > 0) {
	    k+=recvlen;
	    // printf("recv debug: %d\n", recvlen);
	    if(k >= size) break;
	  }
	  // printf("debug recvlen = %d\n", k);
	  if(recvlen < 0) {
	    printf("Error: Got -1 from recvfrom\n");
	    exit(5);
	  }
	  if(size != k) {
	    printf("Error: recvfrom mismatch\n");
	    exit(6);
	  }
		
	  // printf("sending response \"%s\"\n", buf);
	  /* int j; */
	  /* for(j = 0; j < size; j+= MAX_PACKET_SIZE) { */
	  /*   if (sendto(fd, &sendbuff[j], MAX_PACKET_SIZE, 0, (struct sockaddr *)&remaddr, addrlen)==-1) { */
	  /*     perror("sendto"); */
	  /*     exit(1); */
	  /*   } */
	  /* } */

	  int j = size;
	  int iter = 0;
	  for(; j > 0; j-= MAX_PACKET_SIZE) {
	    if (sendto(fd, &sendbuff[iter], j < MAX_PACKET_SIZE ? j : MAX_PACKET_SIZE, 0, (struct sockaddr *)&remaddr, addrlen)==-1) {
	      perror("sendto");
	      exit(1);
	    }
	    iter+=MAX_PACKET_SIZE;
	    // printf("sending iter=%d\n", iter);
	  }
	}

	// printf("sending done\n");		
		// 		if (sendto(fd, buf, 6500, 0, (struct sockaddr *)&remaddr, addrlen) < 0)
		//	perror("sendto");
		//}
	/* never exits */
	return 0;
}
