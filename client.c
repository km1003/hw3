#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef char bool;

uint64_t getuSecs()
{
  struct timeval tv;
  uint64_t t1;

  gettimeofday(&tv, NULL);
  t1 = (tv.tv_sec * 1000000 + tv.tv_usec);
  return t1;
}

int main(int argc, char* argv[])
{
  char arg_error_str[] = "Usage: client xxx.xxx.xxx.xxx\n";
  char buffer[10] = "0123456789";
  int sock;
  struct sockaddr_in server, client;
  uint64_t start, end;

  // check args
  if(argc < 2){
    printf("%s", arg_error_str);
    return -1;
  }
  if((inet_addr(argv[1]) == INADDR_NONE)){
    printf("%s", arg_error_str);
    return -1;
  }

  // Init
  if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
    printf("socket error = %d\n", sock);
    return -1;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(54321); // server port 54321
  server.sin_addr.s_addr = inet_addr(argv[1]); // server ip taken from args

  client.sin_family = AF_INET;
  client.sin_port = htons(0);
  client.sin_addr.s_addr = htonl(INADDR_ANY);

  if((bind(sock, (struct sockaddr*)&client, sizeof(client))) < 0){
    printf("bind error\n");
    return -1;
  }

  while(1){


  }

  return 0;
}
