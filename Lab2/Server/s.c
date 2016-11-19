#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
// #include <pthread.h>

#define SERVER_PORT 6001
#define BUF_SIZE 40480

//not really required
int delayNano(long t1, long t2)
{
  struct timespec tim, tim2;
   tim.tv_sec = t1;
   tim.tv_nsec = t2;

   if(nanosleep(&tim , &tim2) < 0 )   
   {
      printf("Nano sleep system call failed \n");
      return -1;
   }

   //printf("Nano sleep successfull \n");

   return 0;
}



int main(int argc, char * argv[]){
  struct sockaddr_in sin;
  struct sockaddr_storage client_addr;
  char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
  socklen_t client_addr_len;
  char buf[BUF_SIZE];
  int len;
  int s;
  char *host;
  struct hostent *hp;
  ssize_t bytesSent, bytesRead, totalFileSize;
  int i = 1;
  struct timespec tim, tim2;
  tim.tv_sec = 0.5;
  tim.tv_nsec = 5000000;
  struct timeval t1,t2;
  long sleep;
  // pthread_t attr1,attr2,attr3,attr4,att5;
//payload gets a value from the command line, else it is predefined
long int payLoad /*= ((argv[3] != "\0") ? argv[3] : BUF_SIZE)*/; 

if (argc == 4)
{
  /* code */
  payLoad = atol(argv[3]); //get the payload size from the command line itself
}

else {
  payLoad = BUF_SIZE;
}

if(argc == 5)
  sleep =  atol(argv[4]); //get sleep duration from the command line
	//tim.tv_nsec = sleep;
  printf("The buffer size you've entered is %ld\n", payLoad);
  printf("The sleep duration you have entered is %ld\n",sleep);

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
  
  printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);
    
  
  client_addr_len = sizeof(client_addr);
  // gettimeofday(&t1,NULL);
  /* Receive messages from clients*/
  while(len = recvfrom(s, buf, sizeof(buf), 0,
		       (struct sockaddr *)&client_addr, &client_addr_len)){
  	i=1;
  	if(argc == 3) { //getting the file name from the command line
	    fp = fopen(argv[2], "r");  //opening it in reading mode
	    if (fp == NULL) {
	      fprintf(stderr, "Error opening file\n");
	      exit(1);
	    }
  	}
    
    inet_ntop(client_addr.ss_family,
	      &(((struct sockaddr_in *)&client_addr)->sin_addr),
	      clientIP, INET_ADDRSTRLEN);      //converting clientIP into a string
    
    printf("Server got message from %s: %s [%d bytes]\n", clientIP, buf, len);
    totalFileSize=0;
    if(strcmp(buf,"GET\n")==0){
      printf("Server received a GET from the client!\n");
      while((bytesRead = fread(buf,1,40000/*payLoad(size_t)atol(argv[3])*/,fp)) > 0 ){//hardcoding works here
      	
      	if((bytesSent = sendto(s, buf, bytesRead, 0, (struct sockaddr*)&client_addr, client_addr_len) ) < bytesRead){
      		perror("File not sent \n");
      		return -1;
      	}
        //delayNano(0,100000000000);
      	totalFileSize += bytesSent;
      	printf("Loop Number: %d Bytes Read: %d , Bytes Sent: %d , Total Bytes sent : %d \n",i,bytesRead,bytesSent,totalFileSize);
      	i++;
      	nanosleep(&tim,&tim2);
      }

    }
    //puts("FILE SENT !");
    fclose(fp);
    fflush(fp);
    printf("TEST\n"); //testing
  /* Send BYE to signal termination */
  	strcpy(buf, "BYE");
  //puts("In the end");
  	sendto(s, buf, sizeof(buf), 0, 
	 (struct sockaddr*)&client_addr, client_addr_len);
  	printf("File Sent!\n"); //nothing gets executed after this!
  }
  puts("TEST \n"); //testing
  memset(buf, 0, sizeof(buf));
  // exit(0);//test exit
  
  exit(0);
  //gettimeofday(&t2,NULL);


  // int elapsedTime;
  // int dataRate;

  // elapsedTime = (int)(t2.tv_sec - t1.tv_sec)*1000;
  // elapsedTime+=(int)(t2.tv_usec - t1.tv_usec)/1000;

  // printf("Elapsed time \t%d", elapsedTime);
  // dataRate = sizeof(buf)/elapsedTime;
  // printf("Data rate : \t%d",dataRate);
}

