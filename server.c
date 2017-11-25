#include "headers.h"

// 
int DEBUG_MODE = 1;
struct clients{
int list[MAX_CLIENTS];
int size;
};
FILE* file ;
struct clients clients_list;
int main(int argc, char *argv[])
{
//puts("1234");
//fprintf(stdout,"123\n");
//file = fopen("./log.txt", "a");
//perror("file");
//fprintf(stdout,"123\n");

//freopen("log.txt", "a", stderr);
//fprintf(stdout,"123\n");
char* str;
memset(&clients_list,0,MAX_CLIENTS);
 
  if(argc > 1) DEBUG_MODE = 0;

 if(DEBUG_MODE){
  //fprintf(stdout,"123\n");
     file = fopen(LOG_FILE_NAME, "a");
     fprintf(file,"server: Debug mode is ON!\n");
     
     fprintf(file,"server: MAIN: argc = %d\n", argc);
     
     for(int i=0; i<argc; i++){
      //file = fopen("./log.txt", "a");
      fprintf(file,"server:  argv[%d] = %s\n", i, argv[i]);
     }
     fclose(file);
         
 }else 
 {
  file = fopen("./log.txt", "a");
  fprintf(file,"server: Debug mode is OFF!\n");
  fclose(file);
}
 
 int listener;

 
 struct sockaddr_in addr, their_addr;
 // Заполнение структуры адреса сервера
 addr.sin_family = PF_INET;
 addr.sin_port = htons(SERVER_PORT);
 addr.sin_addr.s_addr = htonl(SERVER_HOST);

//  //  размер структуры для передачи в функцию accept
 socklen_t socklen;
 socklen = sizeof(struct sockaddr_in);

 // создаем ev - события слушающего сокета и events - события от сокетов клиентов
 static struct epoll_event ev, events[EPOLL_SIZE];
 // есть ли приходящие сообщения   
 ev.events = EPOLLIN |EPOLLET;//EPOLLET - отслеживаем, только если произошли изменения 

 // массив для сообщения в чате
 char message[BUF_SIZE];

 // дескриптор epoll
 int epfd;

 // client -  дескриптор нового клиента
 // res - переменная для хранения кодов возврата
 // epoll_events_count - число изменений в отслеживаемых сокетах ;
 int client, res, epoll_events_count;


 
 // создание слушающего сокета
 CHK2(listener, socket(PF_INET, SOCK_STREAM, 0));
 file = fopen("./log.txt", "a");
 fprintf(file,"server: Main listener(fd=%d) created! \n",listener);
 fclose(file);

 // неблокирующий режим работы сокета(управление возвращается сразу же)
 setnonblocking(listener);

 // закрепление за портом 
 CHK(bind(listener, (struct sockaddr *)&addr, sizeof(addr)));
 file = fopen("./log.txt", "a");
 fprintf(file,"server: Listener binded to: %d\n", SERVER_HOST);
 fclose(file);

 // слушаем
 CHK(listen(listener, MAX_CLIENTS));
 file = fopen("./log.txt", "a");
 fprintf(file,"server: Start to listen: %d!\n", SERVER_HOST);
 fclose(file);

 // создание декстриптора объекта epoll
 CHK2(epfd,epoll_create(EPOLL_SIZE));
 file = fopen("./log.txt", "a");
 fprintf(file,"server: Epoll(fd=%d) created!\n", epfd);
 fclose(file);

 //  ev - для слушающего сокета, добавляем его туда
 ev.data.fd = listener;

 // добавляем слушающие сокет в epoll
 CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev));
 file = fopen("./log.txt", "a");
 fprintf(file,"server: Main listener(%d) added to epoll!\n", epfd);
 fclose(file);
// принимаем сообщения, подключаем клиентов
 while(1)
 {
     CHK2(epoll_events_count,epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_RUN_TIMEOUT));
     if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Epoll events count: %d\n", epoll_events_count);fclose(file);
     
     for(int i = 0; i < epoll_events_count ; i++)
     {
             if(DEBUG_MODE){
              file = fopen("./log.txt", "a");
                     fprintf(file,"server: events[%d].data.fd = %d\n", i, events[i].data.fd);
                     fclose(file);

             }
             // если изменение в слушающем сокете - подключаем новый клиент
             if(events[i].data.fd == listener)
             {
                     CHK2(client,accept(listener, (struct sockaddr *) &their_addr, &socklen));
                     if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: connection from:%s:%d, socket assigned to:%d \n",
                                       inet_ntoa(their_addr.sin_addr),
                                       ntohs(their_addr.sin_port),
                                       client);fclose(file);
                     
                     setnonblocking(client);

                    
                     ev.data.fd = client;

                     
                     CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev));

                     
                     clients_list.list[clients_list.size] = client;
                     clients_list.size++; // добавляем в список клиентов
                     if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Add new client(fd = %d) to epoll and now clients_list.size = %d\n", 
                                         client,  
                                         clients_list.size);fclose(file);

                     
                     memset(&message,0,BUF_SIZE);//bzero(message, BUF_SIZE);
                     res = sprintf(message, STR_WELCOME, client);
                     CHK2(res, send(client, message, BUF_SIZE, 0));

             }else {
                     CHK2(res,handle_message(events[i].data.fd));
             }
     }     
 }

 close(listener);
 close(epfd);
 fflush(file);
fclose(file);
 return 0;
}

// обработка изменений в сокете клиента
int handle_message(int client)
{
 // buf -сообщение клиента, message - форматированная строка для рассылки другим процессорам
file = fopen("./log.txt", "a");
//freopen("log.txt", "a", stderr);
char* str;

 char buf[BUF_SIZE], message[BUF_SIZE];
 memset(&buf,0,BUF_SIZE);
 memset(&message,0,BUF_SIZE);

 // для хранения возвращаемого значения 
 int len;

 // получение сообщения
 if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Try to read from fd(%d)\n", client);fclose(file);
 CHK2(len,recv(client, buf, BUF_SIZE, 0));

 // если ничего не считали - клиент отключился 
 if(len == 0){
     CHK(close(client));
     delete_element(&clients_list.list[0],client,&clients_list.size);
     if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Client with fd: %d closed! And now clients_list.size = %d\n", client, clients_list.size);fclose(file);
 // рассылка сообщения остальным клиентам
 }else{

     if(clients_list.size == 1) { // никто кроме нас не подключен
             CHK(send(client, STR_NOONE_CONNECTED, strlen(STR_NOONE_CONNECTED), 0));
             return len;
     }
    
     // Строка для рассылки остальным клиентам
     sprintf(message, STR_MESSAGE, client, buf);

     int i=0;
     for(i=0;i<clients_list.size;i++){
             CHK(send(clients_list.list[i], message, BUF_SIZE, 0));
             if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Message '%s' send to client with fd(%d) \n", message, clients_list.list[i]);fclose(file);
        }
     
     if(DEBUG_MODE) file = fopen("./log.txt", "a");fprintf(file,"server: Client(%d) received message successfully:'%s', a total of %d bytes data.\n",client,buf,len);fclose(file);
 
}
  return 0;
}
