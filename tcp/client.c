#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/tcp.h>
// #define _GNU_SOURCE
#include <sched.h>

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
  int sockfd = 0, n = 0;
  char sendBuff[512*1024+1], recvBuff[512*1024+1];
  struct sockaddr_in serv_addr;

  setcpuaffinity();

  if(argc != 5)
    {
      printf("\n Usage: %s <ip of server> \n",argv[0]);
      return 1;
    }

  int size = atoi(argv[3]);
  FILE *fp = fopen(argv[2], "r");

  fread(&sendBuff, size, sizeof(char), fp);
  sendBuff[size] = 0;
  
  memset(recvBuff, '0',sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  // if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    }

  setsocket_options(sockfd);

  memset(&serv_addr, '0', sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8091); 

  if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
      printf("\n inet_pton error occured\n");
      return 1;
    } 

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("\n Error : Connect Failed \n");
      return 1;
    }

  //n = read(sockfd, recvBuff, sizeof(recvBuff));
  //recvBuff[n] = 0;
  // printf("debug: %s n=%d\n", recvBuff, n);
  //printf("%s\n", recvBuff);
  int maxIters = atoi(argv[4]);
  int counter = 0;
  while(counter < maxIters) {
    int count = 0;
    while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0) {

      recvBuff[n] = 0;
      count+=n;
      if(count >= size) break;
      
    }
    if(n < 0)
      {
	printf("\n Read error \n");
      }
    if(count != size) {
      printf("Read error mismatch\n");
    }

    n = write(sockfd, recvBuff, size);
    if(n != size) {
      printf("write error. size mismatch\n");
    }
    counter++;
  }
  printf("writing done\n");
  close(sockfd);
  return 0;
}
