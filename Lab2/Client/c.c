#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#define SERVER_PORT 6001
#define BUF_SIZE 40480


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

  if(argc == 3) {           //if the file name has been provided as well, set the file pointer to open that file
    fp = fopen(argv[2], "w");
    if (fp == NULL) {
      fprintf(stderr, "Error opening output file\n");
      exit(1);
    }
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

  gettimeofday(&t1,NULL);
  

  /* send message to server */  
  fgets(buf, sizeof(buf), stdin);
  buf[BUF_SIZE-1] = '\0';
  len = strlen(buf) + 1;
  if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {     //using sendto() instead of send() for implementing UDP
    perror("Client: sendto()"); 
    return 0; 
  }
    
  /* get reply, display it or store in a file*/ 
  /* Add code to receive unlimited data and either display the data
     or if specified by the user, store it in the specified file. 
     Instead of recv(), use recvfrom() call for receiving data */
  
  //fputs(buf, stdout);
  
  addrlen = sizeof(sin);

  while((rcv = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *)&sin, &addrlen) )>0) {  //as long as the file is being received successfully
     printf("\n");
     printf("\nLoop Number: %d Received Bytes : %d \n",i,rcv);
     printf("\n");
     i++;
     if(strcmp("BYE",buf)==0){
        printf("File Received \n");
        printf("Client Closing \n");
        //fclose(fp);
        //exit(0);
        break;
     }
     //printf(buf);
     fflush(fp);
     if(fwrite(buf,1,rcv,fp)<0){
        perror("Error Writing to File");
     }
    
    
     
  }
  fclose(fp);
  gettimeofday(&t2,NULL);

  //the elapsed time is calculated as the difference between the two gettimeofday() calls 
  elapsedTime = (int)(t2.tv_sec - t1.tv_sec)*1000;
  elapsedTime+=(int)(t2.tv_usec - t1.tv_usec)/1000;

  printf("Elapsed time %d ms\n", elapsedTime);
  dataRate = sizeof(buf)/elapsedTime;
  printf("Data rate : %dbytes/ms\n",dataRate);

  exit(0);

}
