#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <malloc.h>

#define SERVER_PORT 6001
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
  
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[BUF_SIZE];
  int s,i=1;
  int len;
  ssize_t rcv;
  socklen_t addrlen;

  if ((argc==2)||(argc == 3)) {       //assigns the first command line argument as the chosen IP
    host = argv[1];
  }
  else {
    fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
    exit(1);
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);         //get the host's IP address using the first command line argument
  if (!hp) {
    fprintf(stderr, "client: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Host %s found!\n", argv[1]);

  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);    //copies the memory area from the host address to the socket address
  sin.sin_port = htons(SERVER_PORT);
  

  /* create socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {       //creating a socket for the Internet domain, and for handling file transfer
    perror("client: socket");
    exit(1);
  }

  printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
  printf("To play the music, pipe the download file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n"); 
  
  struct timeval t1,t2;
  int dataRate, elapsedTime;
  ssize_t totalSize = 0,totalTime = 0;
  
  fR_t request;
    /* send message to server */
  printf("Enter file name to be received \n");  
  //fgets(buf, sizeof(buf), stdin);
  scanf("%s",&buf);
  //buf[BUF_SIZE-1] = '\0';
  fp = fopen(buf, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Error opening output file\n");
    exit(1);
  }
  len = strlen(buf) + 1;
  if(DEBUG == 1){
    printf("Len:  %d \n",len);
    printf("Buf: %sx \n",buf);  
  }
  request.type = 0;
  request.filename_size = len;
  strcpy(request.filename,buf);
  memcpy(&buf,&request,sizeof(request));
  if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {     //using sendto() instead of send() for implementing UDP
    perror("Client: sendto()"); 
    return 0; 
  }
    
  /* get reply, display it or store in a file*/ 
  /* Add code to receive unlimited data and either display the data
     or if specified by the user, store it in the specified file. 
     Instead of recv(), use recvfrom() call for receiving data */
    //fputs(buf, stdout);
  int altSeqNumber = 0;
  int firstPack =1;
  // int typex;
  fnf_t notFound;
  fiD fPackRecv;
  rcvD dataToBeRecv;
  ack ackS;
  addrlen = sizeof(sin);
  gettimeofday(&t1,NULL);
  // printf("Waiting to Receive\n");
  while((rcv = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *)&sin, &addrlen))>0){  //as long as the file is being received successfully
    printf("\nLoop Number: %d Received Bytes : %ld \n",i,rcv);
	 i++;
	 totalSize += rcv;
    if(strcmp("BYE",buf)==0){
    	printf("BUF >> %s\n",buf);
        printf("File Received \n");
        printf("Client Closing \n");
    	fclose(fp);
    	break;    
    }

    if(rcv < 53){
    	memcpy(&notFound,&buf,rcv);
    }
    else if(firstPack == 1){
    	memcpy(&fPackRecv,&buf,rcv);	
    }
    else{
    	memcpy(&dataToBeRecv,&buf,rcv);
    }
    
    if(DEBUG == 1){
    	printf("--------------------------\n");
    	printf("BUF >> %s\n",buf);
    	printf("Rcv : %ld \n",rcv);
    	printf("notFound.type : %d \n",notFound.type);
    	printf("fPack.type : %d \n",fPackRecv.type);
    	// typex = fPackRecv.type;
    	printf("dataPack.type : %d \n",dataToBeRecv.type);
    }
    if(notFound.type == 4){
    	printf("File Not Found\n");
    	printf("File Name:%s\n",notFound.filename);
    	printf("Server returned FALSE, Client Exiting!\n");
    	exit(0);
    }
    if(fPackRecv.type == 2){
    	if(DEBUG == 1){
    	   	printf("fPackRecv.sequence_number %d\n",fPackRecv.sequence_number );
    	   	printf("altSeqNumber %d\n",altSeqNumber );
    		printf("Data Block Size: %d\n",fPackRecv.block_size);
    	}
    	firstPack = 0;
    	printf("File Found\n");
    	
     	// totalSize += fPackRecv.block_size;
     	fflush(fp);
     	if(altSeqNumber != fPackRecv.sequence_number){
     		if(fwrite(fPackRecv.data,1,fPackRecv.block_size,fp)<0){
     			perror("Error Writing to File");
     		}	
     	altSeqNumber = 1;
     	}
     	ackS.type = 1;
     	ackS.num_sequences = fPackRecv.sequence_number;
     	memset(buf, 0, sizeof(buf));
     	memcpy(&buf,&ackS,sizeof(ackS));
     	if (sendto(s, buf, sizeof(ackS), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {     //using sendto() instead of send() for implementing UDP
     	  perror("Client: sendto()"); 
     	  return 0; 
     	}
     	fPackRecv.type = 55;
    }
    else if(dataToBeRecv.type == 3){
    	// printf("\nLoop Number: %d Received Bytes : %ld \n",i,rcv);
     // 	i++;
     	// totalSize += dataToBeRecv.block_size;
     	if(DEBUG == 1){
     	    printf("dataToBeRecv.sequence_number %d\n",dataToBeRecv.sequence_number);
     	    printf("altSeqNumber %d\n",altSeqNumber );
     	    printf("Data Block Size: %d\n",dataToBeRecv.block_size);
     	}
     	fflush(fp);
     	if(dataToBeRecv.sequence_number != altSeqNumber){
     		if(fwrite(dataToBeRecv.data,1,dataToBeRecv.block_size,fp)<0){
     			perror("Error Writing to File");
     		}
     		if(altSeqNumber == 1)
     			altSeqNumber = 0;
     		else
     			altSeqNumber = 1;	
     	}

     	ackS.type = 1;
     	ackS.num_sequences = fPackRecv.sequence_number;
     	memset(buf, 0, sizeof(buf));
     	memcpy(&buf,&ackS,sizeof(ackS));
     	if (sendto(s, buf, sizeof(ackS), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {     //using sendto() instead of send() for implementing UDP
     	  perror("Client: sendto()"); 
     	  return 0; 
     	}
    	memset(&dataToBeRecv, 0, sizeof(dataToBeRecv));
    }
    	
    }
		
	gettimeofday(&t2,NULL);
    //the elapsed time is calculated as the difference between the two gettimeofday() calls 
    elapsedTime = (t2.tv_sec - t1.tv_sec)*1000000L + (t2.tv_usec - t1.tv_usec);
    // totalTime += elapsedTime;
    printf("Elapsed time %d micro seconds\n", elapsedTime);
    dataRate = (totalSize)/elapsedTime;
    printf("Data rate : %d bytes/micro second\n",dataRate);
  	exit(0);
}
