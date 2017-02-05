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

#define PORTNO 8091

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

void setsocket_options(int socket) {
  int flag = 1;
  int result = setsockopt(socket,            /* socket affected */
			  IPPROTO_TCP,     /* set option at TCP level */
			  TCP_NODELAY,     /* name of option */
			  (char *) &flag,  /* the cast is historical
						cruft */
			  sizeof(int));    /* length of option value */
    
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1024*512+1];
    char recvBuff[1024*512+1];
    time_t ticks;

    //Set cpu affinity
    setcpuaffinity();
    
    int size = atoi(argv[2]);

    FILE *fp = fopen(argv[1], "r");
    fread(&sendBuff, size, sizeof(char), fp);
    
    sendBuff[size] = 0;
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsocket_options(listenfd);
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    printf("scanning done\n");
    // memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORTNO); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    // printf("debug %s\n", sendBuff);

    // Start measurement
    double minValue = INT_MAX;
    double avgValue = 0.0;
    // snprintf(sendBuff, sizeof(sendBuff), argv[1], );
    int counter = 0;
    int maxIters = atoi(argv[3]);
    while(counter < maxIters) {
      struct timespec requestStart, requestEnd;
      clock_gettime(CLOCK_MONOTONIC, &requestStart);

      int n;
      n = write(connfd, sendBuff, strlen(sendBuff));
      if(n != size) {
	printf("\n Read error mismatch size\n");
      }

      int count = 0;
      while((n = read(connfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
	  // recvBuff[n] = 0;
	  count += n;
	  if(count >= size) {
	    break;
	  }
	}

      // End measurement
      clock_gettime(CLOCK_MONOTONIC, &requestEnd);
          // Calculate time it took
      double accum = ( requestEnd.tv_sec - requestStart.tv_sec )*1000000000.0
		  + ( requestEnd.tv_nsec - requestStart.tv_nsec );

      if(accum < minValue) minValue = accum;
      avgValue += accum;
      // printf("count = %d counter=%d minValue=%lf\n", count, counter, minValue);
      if(n < 0)
	{
	  printf("\n Read error \n");
	  perror("issue: ");
	  exit(1);
	}
      if(count != size) {
	printf("Read error mismatch\n");
	exit(2);
      }
      counter++;
    }
    
    
    printf("Took %lf time avg: %lf to send and recv %lu data\n", minValue, avgValue/maxIters, sizeof(recvBuff));
    close(connfd);
    return 0;
}
