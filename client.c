#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <netinet/in.h>
//#include <sys/types.h>

int main()
{
  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  if(sock < 0)
    printf("socket error = %d\n", sock);
  struct sockaddr foo;

  return 0;
}
