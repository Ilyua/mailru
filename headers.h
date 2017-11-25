#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/epoll.h> 
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
   
#define BUF_SIZE 1024
#define SERVER_PORT 12346
#define SERVER_HOST INADDR_ANY
#define LOG_FILE_NAME "log.txt"
 
// Default timeout - http://linux.die.net/man/2/epoll_wait
#define EPOLL_RUN_TIMEOUT -1

// Count of connections that we are planning to handle (just hint to kernel)
#define EPOLL_SIZE 10000

// First welcome message from server
#define STR_WELCOME "Welcome to seChat! You ID is: Client #%d"

// Format of message population
#define STR_MESSAGE "Client #%d>> %s"

// Warning message if you alone in server
#define STR_NOONE_CONNECTED "Noone connected to server except you!"

// Commad to exit
// Commad to exit
#define CMD_EXIT "EXIT"
#define MAX_CLIENTS 1000

#define CHK(eval) if(eval < 0){perror("eval");exit(-1);}
#define CHK2(res, eval) if((res = eval) < 0){perror("eval"); exit(-1);}

int handle_message(int new_fd);// прототип функции

int setnonblocking(int sockfd)
{
//FILE* file = fopen("./log.txt", "a");
   if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) < 0){
   	perror("eval"); 
   	exit(-1);
   }
   //fprintf(file,"FileDesc %d is nonblock\n",sockfd);
   //fclose(file);
   return 1;
}


int delete_element(int * array, int element,int* n){
	int i=0;
	for(i=0;i<*n;i++){
		if (array[i]== element){
			//printf("%s","Element have been founded");
			*n--;
		}

	}
	if(i==*n-1){
		//printf("%s","Not found");
		return -1;
	}
	int j;
	for(j=i;j<*n;j++){
		array[j]=array[j+1];
	}
	return 1;
}
