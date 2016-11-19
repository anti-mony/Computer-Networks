#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
// #include <pthread.h>

#define SERVER_PORT 6000
#define BUF_SIZE 40480
#define DEBUG 0
// #define NUM_SEQUENCES_STATIC 6
#define DATA_BUF_SIZE 36864

typedef struct fileRequest
  {
    uint8_t type; //0
    uint8_t filename_size;
    char filename[50];
  }fR_t;

  typedef struct acknowledgement
  {
    uint8_t type; //1
    uint8_t num_sequences;
  }ack;

  typedef struct fileInfoAndData
  {
    uint8_t type ; //2
    uint16_t sequence_number;
    uint8_t filename_size;
    char filename[50];
    uint32_t file_size;
    uint16_t block_size;
    char data[DATA_BUF_SIZE];
  }fiD;

  typedef struct receivedData
  {
    uint8_t type; //3
    uint16_t sequence_number;
    uint16_t block_size;
    char data[DATA_BUF_SIZE];
  }rcvD;

  typedef struct FileNotFound
  {
    uint8_t type; //4
    uint8_t filename_size;
    char filename[50];
  }fnf_t;

int main(int argc, char * argv[]){
  struct sockaddr_in sin;
  struct sockaddr_storage client_addr;
  char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
  socklen_t client_addr_len;
  char buf[BUF_SIZE];
  char datbuf[DATA_BUF_SIZE];
  int len;
  int s;
  char *host;
  struct hostent *hp;
  ssize_t bytesSent, bytesRead, totalFileSize, bytesReceived, dataLen;
  int i = 1;
   struct timespec tim, tim2;
  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  
  tim.tv_sec = 2;
  tim.tv_nsec = 0;
  int firstPacket = 1;
  // pthread_t attr1,attr2,attr3,attr4,att5;

  // Declarations for file(s) to be sent 
     FILE *fp;

  /* For inserting delays, use nanosleep()
     struct timespec ... */ 
 

  /* To get filename from commandline */
    
   
  /* Create a socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }
 
 
  /* build address data structure and bind to all local addresses*/
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
 
  /* If socket IP address specified, bind to it. */
  if(argc == 2) {
    host = argv[1];
    hp = gethostbyname(host);
    if (!hp) {
      fprintf(stderr, "server: unknown host %s\n", host);
      exit(1);
    }
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  }
  /* Else bind to 0.0.0.0 */
  else
    sin.sin_addr.s_addr = INADDR_ANY;
  
  sin.sin_port = htons(SERVER_PORT);
  
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("server: bind");
    exit(1);
  }
  else{
    /* Add code to parse IPv6 addresses */
    inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);   //converting sin.sin_addr into a character string
    printf("Server is listening at address %s:%d\n", clientIP, SERVER_PORT);
  }
  
  printf("Client needs to send FileName to receive the file %s\n", argv[1]);
    
  
  client_addr_len = sizeof(sin);
  // gettimeofday(&t1,NULL);
  /* Receive messages from clients*/
  
  fR_t reqRecieved ;
  fiD fPack;
  rcvD dataToBeSent;
  ack ackR;
  fnf_t notFound;  
  int altSeqNumber = 1;

  while(len = recvfrom(s, buf, sizeof(buf), 0,
		       (struct sockaddr *)&sin, &client_addr_len)){
  	i=1;
  	memcpy(&reqRecieved,buf,sizeof(buf));
    if(DEBUG == 1){
      printf(" Type :%u \n ",reqRecieved.type);
      printf("Size : %u \n",reqRecieved.filename_size);
      printf("Name: %s \n",reqRecieved.filename);
    }

    inet_ntop(client_addr.ss_family,
        &(((struct sockaddr_in *)&client_addr)->sin_addr),
        clientIP, INET_ADDRSTRLEN);      //converting clientIP into a string

    
      if(reqRecieved.type == 0){
      fp = fopen(reqRecieved.filename, "rb");  //opening it in reading mode
      if (fp == NULL) {
        notFound.type = 4;
        notFound.filename_size = reqRecieved.filename_size;
        strcpy(notFound.filename,reqRecieved.filename);
        memcpy(&buf,&notFound,sizeof(notFound));
        printf("Size of Not Found:%ld\n",sizeof(notFound));
        fprintf(stderr, "File Not Found !\n");
        if((bytesSent = sendto(s, buf, sizeof(notFound), 0, (struct sockaddr*)&sin, sizeof(sin)))==-1)
        {
          perror("FNF:Ack Not Send");
          exit(0);
        }
        exit(0);
      }
      else{
        printf("File Found: %s , Sending Now!\n",reqRecieved.filename);
        //nanosleep(&tim,&tim2);
        //fflush(fp);
          i=1;
          totalFileSize = 0;      
        //fprintf(stderr, "Check OUTSIDE OUT\n");
        while((bytesRead = fread(datbuf,1,DATA_BUF_SIZE,fp)) > 0 ){
          
          //fprintf(stderr,"Check Outside \n");
          
          if(firstPacket == 1){
            //fprintf(stderr,"Check Inside \n");
            fPack.type = 2;
            fPack.sequence_number = 1;
            fPack.filename_size = reqRecieved.filename_size;
            strcpy(fPack.filename,reqRecieved.filename);
            fPack.block_size = bytesRead;
            memcpy(&fPack.data,&datbuf,bytesRead);
            //memset(datbuf, 0, sizeof(datbuf));
            memcpy(&buf,&fPack,sizeof(fPack));
            dataLen = sizeof(fPack);
            //memset(&fPack, 0, sizeof(fPack));
            firstPacket = 0;
            //fflush(fp);
            if(DEBUG == 1){
              printf("Alt Seq No: %d \n",altSeqNumber);
              printf("bytesRead: %ld\n",bytesRead );  
            }            
          }
          else{
            dataToBeSent.type = 3;
            if(altSeqNumber == 1){
              dataToBeSent.sequence_number = 0;
              altSeqNumber = 0;
            }
            else{
             dataToBeSent.sequence_number = 1; 
             altSeqNumber = 1;
            }

            dataToBeSent.block_size = bytesRead;
            memcpy(&dataToBeSent.data,&datbuf,bytesRead);
            memcpy(&buf,&dataToBeSent,sizeof(dataToBeSent));
            dataLen = sizeof(dataToBeSent);
            //memset(datbuf, 0, sizeof(datbuf));
            //fflush(fp);
            if(DEBUG == 1)
              printf("Alt Seq No: %d \n",altSeqNumber);
          }
          while(ackR.type != 1){
                    if((bytesSent = sendto(s, buf, dataLen, 0, (struct sockaddr*)&sin, client_addr_len) ) < 0){
                      perror("File not sent \n");
                      return -1;
                    }
                    if(DEBUG == 1)
                      printf(" bytesSent: %ld\n",bytesSent);
                    
                    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
                        perror("Error");
                    }

                    if((bytesReceived = recvfrom(s, buf, BUF_SIZE, 0,(struct sockaddr *)&sin, &client_addr_len))<0){
                      //timeout
                      printf("Acknowledgement not recieved, resending packet \n");
                      // if((bytesSent = sendto(s, buf, dataLen, 0, (struct sockaddr*)&sin, client_addr_len) ) < 0){
                      //   perror("File not sent \n");
                      //   return -1;
                      // } 
                    }
                    if(bytesReceived > 0)
                      memcpy(&ackR,&buf,bytesReceived);
          }
          if(DEBUG == 1)
            printf("bytesReceived: %ld\n",bytesReceived);
          
          if(ackR.type == 1){
            printf("Ack Recieved! \n");
            bytesReceived = 0;  
          }
          totalFileSize += bytesSent;
          printf("Loop Number: %d Bytes Read: %ld , Bytes Sent: %ld , Total Bytes sent : %ld \n",i,bytesRead,bytesSent,totalFileSize);
          i++;
          fflush(fp);
          ackR.type = 40;  
        }
          
      }
    }
    //fclose(fp);
    fflush(fp);
    /* Send BYE to signal termination */
    strcpy(buf, "BYE");
    sendto(s, buf, sizeof("BYE"), 0, 
   (struct sockaddr*)&sin, client_addr_len);
    printf("File Sent!\n"); //nothing gets executed after this!
    //puts("TEST \n"); //testing
    memset(buf, 0, sizeof(buf));
    exit(0);


  }
}


