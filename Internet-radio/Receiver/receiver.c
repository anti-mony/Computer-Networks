/* CSD 304 Computer Networks, Fall 2016
   Lab 4, multicast receiver
   Team: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define PORT 5432
#define BUF_SIZE 4096
#define SITE_NAME_SIZE 20
#define SITE_DESC_SIZE 50
#define STATION_COUNT 1
#define STATION_NAME_SIZE 20

typedef struct station_info{
  uint8_t station_number;
  uint8_t station_name_size;
  char station_name[ STATION_NAME_SIZE ];
  uint32_t multicast_address;
  uint16_t data_port;
  uint16_t info_port;
  uint32_t bit_rate;
}station_t;

typedef struct site_info{
  uint8_t type ; //10
  uint8_t site_name_size;
  char site_name[SITE_NAME_SIZE];
  uint8_t site_desc_size;
  char site_desc[SITE_DESC_SIZE];
  uint8_t station_count;
  station_t station_list [STATION_COUNT];
}site_t;

typedef struct station_info_request{
  uint8_t type ;//1 
}sireq_t;


int main(int argc, char * argv[]){
  
  int s; /* socket descriptor */
  struct sockaddr_in sin; /* socket struct */
  char *if_name; /* name of interface */
  struct ifreq ifr; /* interface struct */
  char buf[BUF_SIZE];
  char *host;
  int len;
  /* Multicast specific */
  int MC_PORT = 0;
  char *mcast_addr; /* multicast address */
  struct ip_mreq mcast_req;  /* multicast join struct */
  struct sockaddr_in mcast_saddr; /* multicast sender*/
  socklen_t mcast_saddr_len;


  

  /* Add code to take port number from user */
  
  if(argc == 3) {
    if_name = argv[2];
  }
  else
    if_name = "wl01";

  /* create socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("receiver: socket");
    exit(1);
  }

  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(MC_PORT);
  
  
  /*Use the interface specified */ 
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name , if_name, sizeof(if_name)-1);
  
  if ((setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, 
		  sizeof(ifr))) < 0)
    {
      perror("receiver: setsockopt() error");
      close(s);
      exit(1);
    }

  /* bind the socket */
  if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
    perror("receiver: bind()");
    exit(1);
  }
  
  /* Multicast specific code follows */
  
   build IGMP join message structure 
  mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
  mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

  /* send multicast join message */
  if ((setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		  (void*) &mcast_req, sizeof(mcast_req))) < 0) {
    perror("mcast join receive: setsockopt()");
    exit(1);
  }


  
  /* receive multicast messages */  
  printf("\nReady to listen!\n\n");
   
  while(1) {
    
    /* reset sender struct */
    memset(&mcast_saddr, 0, sizeof(mcast_saddr));
    mcast_saddr_len = sizeof(mcast_saddr);
    
    /* clear buffer and receive */
    memset(buf, 0, sizeof(buf));
    if ((len = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr*)&mcast_saddr, 
			&mcast_saddr_len)) < 0) {
      perror("receiver: recvfrom()");
      exit(1);
    }
    fputs(buf, stdout);
       
    /* Add code to send multicast leave request */
  
  }
  
  close(s);
  return 0;

}
