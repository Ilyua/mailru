#include "headers.h"// то, что headers подключаются два раза это нормально?

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

 int write_to_logfile(char* path,char* log_string){
FILE* file = fopen(path, "a");
fprintf(file,"%s",log_string);
fclose(file);
}




 int sendall(int s, char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = send(s, buf+total, len-total, flags);
        if(n == -1) { break; }
        total += n;
    }

    return (n==-1 ? -1 : total);
}

 int recvall(int s, char *buf, int len, int flags)
{
	 int total = 0;
    int n;

    while(total < len)
    {
        n = recv(s, buf+total, len-total, flags);
        if(n == -1) { break; }
        total += n;
    }

    return (n==-1 ? -1 : total);
}