#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#define SERVER_PORT 5432
#define MAX_PENDING 15
#define MAX_LINE 64000
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
    int i = 0;
    static char human_ip[SITE_NAME_SIZE];
    int * mcastaddrShare; //Shared multicast address between Child[TCP] and Parent[UDP]
    pid_t dat,control, fFplay;
    
    struct hostent *hp; 
    char *host;
    char buf[MAX_LINE];
    int s;
    int len;
    struct sockaddr_in sin; /* socket struct */
    char *if_name; /* name of interface */
    struct ifreq ifr; /* interface struct */
    /* Multicast specific */
    char *mcast_addr; /* multicast address */
    struct ip_mreq mcast_req;  /* multicast join struct */
    struct sockaddr_in mcast_saddr; /* multicast sender*/
    socklen_t mcast_saddr_len;
    site_t site;
    mcastaddrShare = mmap(NULL,sizeof(*mcastaddrShare),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1 ,0);
    int *MC_PORT; //Shared port variable between Child[TCP] and Parent[UDP]
    MC_PORT = mmap(NULL,sizeof(*MC_PORT),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1 ,0);
    int *playControl;//Shared playcontrol variable between Child[TCP] and Parent[UDP] to play/pause the stream.
    playControl = mmap(NULL,sizeof(*playControl),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1 ,0);
    *playControl = 1;
    dat = fork();
    if(dat == 0){
        //*ptd = getpid();
        if (argc==2) {
          host = argv[1];
        }
        else {
          fprintf(stderr, "usage: simplex-talk host\n");
          exit(1);
        }
        /* translate host name into peer's IP address */
        hp = gethostbyname(host);
        if (!hp) {
          fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
          exit(1);
        }
        else
          // printf("Client's remote host: %s\n", argv[1]);
        /* build address data structure */
        bzero((char *)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
        bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
        sin.sin_port = htons(SERVER_PORT);
        /* active open */
        if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
          perror("simplex-talk: socket");
          exit(1);
        }
        else
          // printf("Client created socket.\n");

        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
          {
            perror("simplex-talk: connect");
            close(s);
            exit(1);
          }
        else{
          // printf("Client connected.\n");
        }

        sireq_t req ;
        req.type = 1;

        memcpy(buf,&req,sizeof(req));
        len = send(s, buf, sizeof(req), 0);

        len = recv(s,buf,sizeof(buf),0);

        memcpy(&site,buf,len);
        printf("  -->  %s  <--\n",site.site_name);
        printf("%s\n",site.site_desc);
        printf("Number of channels: %d\n\n",site.station_count);
        int i;
        for(i = 0; i < site.station_count; i++){
            printf("Station Number: %d\n", site.station_list[i].station_number);
            printf("Station Name: %s\n", site.station_list[i].station_name);
            *mcastaddrShare = site.station_list[i].multicast_address;
            printf("%d\n",site.station_list[i].multicast_address );
            inet_ntop(AF_INET, &(site.station_list[i].multicast_address), human_ip, INET_ADDRSTRLEN);
            //printf("hgch\n");
            printf("Multicast Address: %s\n", human_ip);
            printf("Data Port: %d\n", site.station_list[i].data_port);
            printf("Info Port: %d\n", site.station_list[i].info_port);
            printf("Bit Rate: %d\n\n", site.station_list[i].bit_rate);
        }
        int stNum;
        printf("Choose the station:\n");
        scanf(" %d",&stNum);
        *MC_PORT = site.station_list[stNum-1].data_port;
        fflush(stdin);
    }
    else{
          wait(NULL);
          if(argc == 3) {
            if_name = argv[2];
          }
          else
            if_name = "wlo1";

          /* create socket */
          if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("receiver: socket");
            exit(1);
          }

          /* build address data structure */
          memset((char *)&sin, 0, sizeof(sin));
          sin.sin_family = AF_INET;
          sin.sin_addr.s_addr = htonl(INADDR_ANY);
          sin.sin_port = htons(*MC_PORT);
          
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

          int MegaByte = 10240;  
          // Increasing the UDP buffer size to 10MegaBytes using setsockopt()
          if ((setsockopt(s, SOL_SOCKET, SO_RCVBUF, (void *)&MegaByte, 
                  sizeof(MegaByte))) < 0)
            {
              perror("receiver: setsockopt() buf error");
              close(s);
              exit(1);
          }


          /* bind the socket */
          if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
            perror("receiver: bind()");
            exit(1);
          }
          
          /* Multicast specific code follows */
           //build IGMP join message structure 
          mcast_req.imr_multiaddr.s_addr = *mcastaddrShare;  //Passing the address according to the station chosen by the user.
          mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);
          //printf("%d\n",*mcastaddrShare);
          /* send multicast join message */
          if ((setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
                  (void*) &mcast_req, sizeof(mcast_req))) < 0) {
            perror("mcast join receive: setsockopt()");
            exit(1);
          }

          FILE *fp;
          fp = fopen("store.mp3","wb+");
          //mcast_saddr_len = sizeof(sin);
          int j =0;
          control = fork();
          if(control == 0){
            /* Process to control the stream with shared variables 
            * runs forever, waiting for user input 
            * changes the value of variable to an appropriate value to start/stop the stream.
            */

            int c;
            while(1){
                fflush(stdin);
                printf("scanf : %d \n",scanf("%d",&c));
                if(c == 0){
                    *playControl = 0;
                }
                if(c == 1){
                    *playControl = 1;
                }
                if(c == 2){
                    *playControl = 2;
                    exit(0);
                }
                fprintf(stdout, "C: %d\n",c);
                fprintf(stdout, "playControl: %d\n",*playControl);
            }
          }
          else{
            fFplay = fork();
            if(fFplay == 0){
                struct timespec tim1,tim2;
                tim1.tv_sec = 4;
                tim1.tv_nsec = 0 ;
                nanosleep(&tim1,&tim2); 
                system("ffplay -i store.mp3");

            }else{

                while(1) {
                /*Container Loop*/
                if(*playControl == 2){
                        printf("Thank You! Exiting...");
                        exit(0);
                    }
                    /*Actual Receiving Loop managed by shared variable to support play/pause*/
                while(*playControl == 1){
                    if(*playControl == 2){
                        printf("Thank You! Exiting...");
                        exit(0);
                    }
                    /* reset sender struct */
                    memset(&mcast_saddr, 0, sizeof(mcast_saddr));
                    mcast_saddr_len = sizeof(mcast_saddr);
                    if ((len = recvfrom(s, buf, MAX_LINE, 0, (struct sockaddr*)&mcast_saddr,&mcast_saddr_len)) < 0) {
                        
                        
                      perror("receiver: recvfrom()");
                      exit(1);
                    }
                    for(j=0;j<len;j++){
                        // printf("%c",buf[j]); 
                        // fflush(stdout);
                        if(fprintf(fp,"%c",buf[j])<0){
                        perror("Error Writing to File");
                        }
                    }
                    fflush(fp);
                    }
                }
            }
        }
    }
}