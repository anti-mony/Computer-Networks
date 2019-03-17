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
#include <sys/time.h>
#include <fcntl.h> 
#include <time.h>


#define SERVER_PORT 5432
#define MAX_PENDING 15
#define MAX_LINE 64000
#define SITE_NAME_SIZE 20
#define SITE_DESC_SIZE 50
#define STATION_COUNT 4
#define STATION_NAME_SIZE 20
#define PORT_ST1 6000
#define PORT_ST2 6001
#define PORT_ST3 6002
#define PORT_ST4 6003

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
	
	int len;
	int s, new_s;
	char str[INET_ADDRSTRLEN];
	struct sockaddr_in sin;
	struct sockaddr_storage client_addr;
  	char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
	socklen_t client_addr_len;
	char buf[MAX_LINE];
	char *host;
	struct hostent *hp;
	int i = 1;
	struct timespec tim, tim2;
	tim.tv_sec =  1;
  	tim.tv_nsec = 0; //50000000L ;
  	ssize_t bytesSent, bytesRead, totalFileSize;

  	
  	pid_t c1,c2,c3,c4;
  	c1 = fork();
  	if(c1 == 0){
		if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		  //perror("server: socket");
  			exit(1);
  		}
  		char ipP[] = "239.192.24.0";
		/* build address data structure */
  		memset((char *)&sin, 0, sizeof(sin));
  		sin.sin_family = AF_INET;
  		sin.sin_addr.s_addr = inet_addr(ipP);
  		sin.sin_port = htons(PORT_ST1);
  		char fileName[] = "1.mp3";

  		FILE *fp;
  		fp = fopen(fileName,"rb");

  		while((bytesRead = fread(buf,1,MAX_LINE,fp)) > 0 ){
			if((bytesSent = sendto(s, buf, bytesRead, 0, (struct sockaddr*)&sin, sizeof(sin)) ) < 0){
			  //perror("File not sent \n");
  			}
  			i++;
  			if(feof(fp)){
  				rewind(fp);
			}

  			nanosleep(&tim,&tim2);
  		}


		//printf("PID C1: %d\n",getpid());
		//printf("PARENT PID C1: %d\n",getppid());
  	}
  	else{
  		c2 = fork();
  		if(c2 == 0){
  			if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
			  //perror("server: socket");
  				exit(1);
  			}
  			char ipP[] = "239.192.24.0";
			/* build address data structure */
  			memset((char *)&sin, 0, sizeof(sin));
  			sin.sin_family = AF_INET;
  			sin.sin_addr.s_addr = inet_addr(ipP);
  			sin.sin_port = htons(PORT_ST2);
  			char fileName[] = "K.mp3";

  			FILE *fp;
  			fp = fopen(fileName,"rb");

  			while((bytesRead = fread(buf,1,MAX_LINE,fp)) > 0 ){
				if((bytesSent = sendto(s, buf, bytesRead, 0, (struct sockaddr*)&sin, sizeof(sin)) ) < 0){
				  //perror("File not sent \n");
  				}
  				i++;
  				if(feof(fp)){
  					rewind(fp);
				}

  				nanosleep(&tim,&tim2);
  			}

			//printf("PID C2: %d\n",getpid());
			//printf("PARENT PID C2: %d\n",getppid());	
  		}	
  		else{
  			c3 = fork();
  			if(c3 ==0){

  				if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				  //perror("server: socket");
  					exit(1);
  				}
  				char ipP[] = "239.192.24.0";
				/* build address data structure */
  				memset((char *)&sin, 0, sizeof(sin));
  				sin.sin_family = AF_INET;
  				sin.sin_addr.s_addr = inet_addr(ipP);
  				sin.sin_port = htons(PORT_ST3);
  				char fileName[] = "P.mp3";

  				FILE *fp;
  				fp = fopen(fileName,"rb");

  				while((bytesRead = fread(buf,1,MAX_LINE,fp)) > 0 ){
					if((bytesSent = sendto(s, buf, bytesRead, 0, (struct sockaddr*)&sin, sizeof(sin)) ) < 0){
					  //perror("File not sent \n");
  					}
  					i++;
  					if(feof(fp)){
  						rewind(fp);
					}

  					nanosleep(&tim,&tim2);
  				}
				//printf("PID C3: %d\n",getpid());
				//printf("PARENT PID C3: %d\n",getppid());
  			}
  			else{
  				c4 = fork();
  				if(c4 == 0){

  					if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
						  //perror("server: socket");
  						exit(1);
  					}
  					char ipP[] = "239.192.24.0";
						/* build address data structure */
  					memset((char *)&sin, 0, sizeof(sin));
  					sin.sin_family = AF_INET;
  					sin.sin_addr.s_addr = inet_addr(ipP);
  					sin.sin_port = htons(PORT_ST4);
  					char fileName[] = "G.mp3";

  					FILE *fp;
  					fp = fopen(fileName,"rb");

  					while((bytesRead = fread(buf,1,MAX_LINE,fp)) > 0 ){
						if((bytesSent = sendto(s, buf, bytesRead, 0, (struct sockaddr*)&sin, sizeof(sin)) ) < 0){
							  //perror("File not sent \n");
  						}
  						i++;
  						if(feof(fp)){
  							rewind(fp);
						}

  						nanosleep(&tim,&tim2);
  					}
					//printf("PID C4: %d\n",getpid());
					//printf("PARENT PID C4: %d\n",getppid());
  				}
  				else{
					//printf("PARENT: %d\n",getpid());
					//printf("PARENT PID: %d\n",getppid());
					
					/* build address data structure */
  					sireq_t req;
  					site_t mySite;

  					mySite.type = 10;
  					strcpy(mySite.site_name,"Dr. X Radio");
  					mySite.site_name_size = sizeof("Dr. X Radio");
  					strcpy(mySite.site_desc,"Hey! Welcome to Dr.X Radio Show.");
  					mySite.site_desc_size = sizeof("Hey! Welcome to Dr.X Radio Show.");
  					mySite.station_count = 4;

  					mySite.station_list[0].station_number = 1;
  					strcpy(mySite.station_list[0].station_name,"Alt-J");
  					mySite.station_list[0].station_name_size = sizeof("Alt-J");
  					mySite.station_list[0].multicast_address = inet_addr("239.192.24.0");
  					mySite.station_list[0].data_port = 6000; 
  					mySite.station_list[0].info_port = 5432;
  					mySite.station_list[0].bit_rate = 360 * 1000;

  					mySite.station_list[1].station_number = 2;
  					strcpy(mySite.station_list[1].station_name,"Kodaline");
  					mySite.station_list[1].station_name_size = sizeof("Kodaline");
  					mySite.station_list[1].multicast_address = inet_addr("239.192.24.0");
  					mySite.station_list[1].data_port = 6001; 
  					mySite.station_list[1].info_port = 5432;
  					mySite.station_list[1].bit_rate = 360 * 1000;

  					mySite.station_list[2].station_number = 3;
  					strcpy(mySite.station_list[2].station_name,"Porcupine Tree");
  					mySite.station_list[2].station_name_size = sizeof("Porcupine Tree");
  					mySite.station_list[2].multicast_address = inet_addr("239.192.24.0");
  					mySite.station_list[2].data_port = 6002; 
  					mySite.station_list[2].info_port = 5432;
  					mySite.station_list[2].bit_rate = 960 * 1000;

  					mySite.station_list[3].station_number = 4;
  					strcpy(mySite.station_list[3].station_name,"Guns and Roses");
  					mySite.station_list[3].station_name_size = sizeof("Guns and Roses");
  					mySite.station_list[3].multicast_address = inet_addr("239.192.24.0");
  					mySite.station_list[3].data_port = 6003; 
  					mySite.station_list[3].info_port = 5432;
  					mySite.station_list[3].bit_rate = 360 * 1000;					

  					bzero((char *)&sin, sizeof(sin));
  					sin.sin_family = AF_INET;
  					sin.sin_addr.s_addr = inet_addr(argv[1]);
  					sin.sin_port = htons(SERVER_PORT);
					/* setup passive open */
  					if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
					  //perror("simplex-talk: socket");
  						exit(1);
  					}

  					inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  					printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

  					if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
					  //perror("simplex-talk: bind");
  						exit(1);
  					}
  					else
  						printf("Internet Radio Started!\n");

  					listen(s, MAX_PENDING);
					/* wait for connection, then receive and print text */
  					while(1) {
  						if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
					    //perror("simplex-talk: accept");
  							exit(1);
  						}
  						printf("New user connected!.\n");
  						len = recv(new_s, buf, sizeof(buf), 0);	
  						if(len == 1){
  							memcpy(&req,buf,len);
  						}
  						if(req.type == 1){
  							memcpy(buf,&mySite,sizeof(mySite));
  							send(new_s,buf,sizeof(mySite),0);
  						}
					}
  				}
  			}
  		}	
  	}	
}




