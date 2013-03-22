#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 54321

#define true 1
#define false 0
typedef char bool;

#define BUF_SIZE 1024
char buf[BUF_SIZE];

int main(int argc, char* argv[])
{
  char arg_error_str[] = "Usage: server P, where 0.0 < P <= 1.0\n";
  float P;
  int sock;
  struct sockaddr_in server, client;

  // check args
  if(argc < 2)
  {
    printf("%s", arg_error_str);
    return -1;
  }
  P = atof(argv[1]);
  if(P <= 0.0 || P > 1.0)
  {
    printf("%s", arg_error_str);
    return -1;
  }

  // Init
  if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
  {
    printf("socket error = %d\n", sock);
    return -1;
  }
  int flags = fcntl(sock, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(sock, F_SETFL, flags);
  
  // server address
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // client address
  client.sin_family = AF_INET;
  client.sin_port = htons(0);
  client.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind server to socket
  if((bind(sock, (struct sockaddr*)&server, sizeof(server))) < 0)
  {
    printf("bind error\n");
    return -1;
  }

  printf("server listing on port %d\n", PORT);

  while(1)
  {
    // check for received packet
    socklen_t addrlen;
    int bytes_rcvd = recvfrom(sock, buf, BUF_SIZE, 0,
      (struct sockaddr*)&client, &addrlen);

    if(bytes_rcvd > 0)
    {
      // recover the integer ID from the udp data
      int packet_id = atoi(buf);
      if(packet_id == 0)
        printf("warning invalid packet ID ");
      printf("[%d|%d] ", packet_id, bytes_rcvd);
      long ip = client.sin_addr.s_addr;
      printf("%d.%d.%d.%d:%d", (int)(ip & 0xff), (int)((ip >> 8) & 0xff), (int)((ip >> 16) & 0xff),
        (int)((ip >> 24) & 0xff), ntohs(client.sin_port));

      // send a response with the probability of P
      float roll = ((float)rand())/RAND_MAX;

      // send a response
      if(roll <= P)
      {
        int bytes_sent = sendto(sock, buf, bytes_rcvd, 0,
          (struct sockaddr*)&client, sizeof(client));
        printf(" roll:%f, P:%f, sent %d bytes\n", roll, P, bytes_sent);
      }
      // don't send a reponse
      else
      {
        printf(" roll:%f, P:%f, not sending\n", roll, P);
      }

    }
    
  } // end while

  return 0;
}

