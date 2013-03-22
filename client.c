#include <fcntl.h>
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

// tick period in milliseconds, how fast to send packets
#define TICK_PERIOD 1500

#define true 1
#define false 0
typedef char bool;

char buf[32];

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
  bool check = false;
  bool tock = false;
  char arg_error_str[] = "Usage: client xxx.xxx.xxx.xxx\n";
  char payload[32];
  int count, num_rcvd, sock;
  struct sockaddr_in server, client, response;
  uint64_t start, end, tick, rtt_sum;

  // check args
  if(argc < 2)
  {
    printf("%s", arg_error_str);
    return -1;
  }
  if((inet_addr(argv[1]) == INADDR_NONE))
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
  count = 0;
  num_rcvd = 0;
  rtt_sum = 0;
  sprintf(payload, "%31d", count);
  
  // server address
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr(argv[1]); // server ip taken from args

  // client address
  client.sin_family = AF_INET;
  client.sin_port = htons(0);
  client.sin_addr.s_addr = htonl(INADDR_ANY);

  // reponse address
  response.sin_family = AF_INET;
  response.sin_port = htons(0);
  response.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind client to socket
  if((bind(sock, (struct sockaddr*)&client, sizeof(client))) < 0)
  {
    printf("bind error\n");
    return -1;
  }

  printf("sending udp packets to %s:%d", argv[1], PORT);

  while(1)
  {
    // create a tick to time sending packets to server
    tick = getuSecs();
    if(!((tick/1000) % TICK_PERIOD))
    {
      if(!tock)
      {
        // detect packet loss: end timestamp was never set by server response
        if(count > 0 && start == end)
        {
          printf("timeout, rcvd %d out of %d", num_rcvd, count);
        }
        else if(count > 0 && start < end)
        {
//          float rtt, avgrtt;
//          rtt = (end-start)/1000.0;
//          avgrtt = (((float)rtt_sum)/num_rcvd)/1000.0;
//          // the last sent packet got a reponse, output stats
//          printf(" rtt:%.3fms, avgerage rtt:%.3fms, rcvd %d out of %d",
//            rtt, avgrtt, num_rcvd, count);

          // the last sent packet got a reponse, output stats
          printf(" rtt:%lluus, avgrtt:%.3fus, rcvd %d out of %d",
            (end-start), ((float)rtt_sum)/num_rcvd, num_rcvd, count);

        }
        count++;
        tock = true;
        printf("\n[%3d] ", count);
        // put count as ascii string in payload
        sprintf(payload, "%31d", count);
        // send a udp packet to the server
        int bytes_sent = sendto(sock, payload, strlen(payload), 0,
          (struct sockaddr*)&server, sizeof(server));
        if(bytes_sent != strlen(payload))
        {
          printf("warning: sendto returned [%d] not [%d]",
            bytes_sent, strlen(payload));
        }
        // timestamp
        start = tick;
        end = tick;
      }
    } else tock = false;

    // check for received packet
    socklen_t addrlen;
    int bytes_rcvd = 0;
    // check every 10 microseconds
    if(count > 0 && !((tick/10) % 10))
    {
      if(!check)
      {
        bytes_rcvd = recvfrom(sock, buf, strlen(payload), 0,
          (struct sockaddr*)&response, &addrlen);
        check = true;
//        printf(".");
      }
    } else check = false;

    // a packet was received
    if(bytes_rcvd == strlen(payload))
    {
      // check if this is the response to the last packet we sent
      int id = atoi(buf);
      if(id == count)
      {
        // got server response, timestamp it and increment num_rcvd
        num_rcvd++;
        end = tick;
        rtt_sum += (end - start);
      }
      else // packet is from a previous count value, old packet
      {
        printf("warning got packet [%d] expected [%d], rtt avg is invalid",
          id, count);
      }
    }
    else if(bytes_rcvd > 0) // incomplete packet received
    {
      printf("warning rcvd %d bytes instead of %d ",
        bytes_rcvd, strlen(payload));
      int i;
      for(i = 0; i < bytes_rcvd; i++)
        printf("%c", buf[i]);
    }
    
  } // end while

  return 0;
}

