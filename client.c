#include "headers.h"


// chat message buffer
char message[BUF_SIZE];

// for debug mode
int DEBUG_MODE = 1;

/*
  We use 'fork' to make two process.
    Child process:
    - waiting for user's input message;
    - and sending all users messages to parent process through pipe.
    ('man pipe' has good example how to do it)

    Parent process:
    - wating for incoming messages(EPOLLIN):
    -- from server(socket) to display;
    -- from child process(pipe) to transmit to server(socket)
     
*/

int main(int argc, char *argv[])
{
if(argc > 1) DEBUG_MODE = 0;

if(DEBUG_MODE){
        printf("Debug mode is ON!\n");
        printf("MAIN: argc = %d\n", argc);
        int i=0;
        for(i=0; i<argc; i++)
                printf(" argv[%d] = %s\n", i, argv[i]);
}else printf("Debug mode is OFF!\n");

int sock, pid, pipe_fd[2], epfd;


struct sockaddr_in addr;
addr.sin_family = PF_INET;
addr.sin_port = htons(SERVER_PORT);
addr.sin_addr.s_addr = htonl(SERVER_HOST);


static struct epoll_event ev, events[2]; // Socket(in|out) & Pipe(in)
ev.events = EPOLLIN |EPOLLET;

// если ноль - закрываем клиент
int continue_to_work = 1;


CHK2(sock,socket(PF_INET, SOCK_STREAM, 0));
CHK(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0);

// pipe -для общения двух созданных процессорв
CHK(pipe(pipe_fd));
if(DEBUG_MODE) printf("Created pipe with pipe_fd[0](read part): %d and pipe_fd[1](write part): % d\n",
                pipe_fd[0],
                pipe_fd[1]);

CHK2(epfd,epoll_create(EPOLL_SIZE));
if(DEBUG_MODE) printf("Created epoll with fd: %d\n", epfd);

//  добавление сокета ,слушающего сообщения от сервера
ev.data.fd = sock;
CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev));
if(DEBUG_MODE) printf("Socket connection (fd = %d) added to epoll\n", sock);

// pipe_fd[0]-слушает сообщения ребенка
ev.data.fd = pipe_fd[0];
CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fd[0], &ev));
if(DEBUG_MODE) printf("Pipe[0] (read) with fd(%d) added to epoll\n", pipe_fd[0]);


CHK2(pid,fork());
switch(pid){
        case 0: // child process
                close(pipe_fd[0]); // pipe[0] - нам больше не нужен , мы не будем ничего читать
                printf("Enter 'exit' to exit\n");
                while(continue_to_work){
                        bzero(&message, BUF_SIZE);
                        fgets(message, BUF_SIZE, stdin);

                        // сравнение двух строк без учета регистра 
                        if(strncasecmp(message, CMD_EXIT, strlen(CMD_EXIT)) == 0){
                                continue_to_work = 0;
                                // посылаем сообщение родителю для рассылки остальным процессам
                        }else CHK(write(pipe_fd[1], message, strlen(message) - 1));
                }
                break;
        default: //parent process
                close(pipe_fd[1]); // писать больше не будем

                
                int epoll_events_count, res;

                // *** Main cycle(epoll_wait)
                while(continue_to_work) {
                        CHK2(epoll_events_count,epoll_wait(epfd, events, 2, EPOLL_RUN_TIMEOUT));
                        if(DEBUG_MODE) printf("Epoll events count: %d\n", epoll_events_count);

                        for(int i = 0; i < epoll_events_count ; i++){
                                bzero(&message, BUF_SIZE);

                                // EPOLLIN event from server// почему обновляется событие epollin, если сервер ничего не прислал, а только закрылся?
                                if(events[i].data.fd == sock){
                                        if(DEBUG_MODE) printf("Server sends new message!\n");
                                        CHK2(res,recv(sock, message, BUF_SIZE, 0));

                                        // сервер закрыл соединение 
                                        if(res == 0){
                                                if(DEBUG_MODE) printf("Server closed connection: %d\n", sock);
                                                CHK(close(sock));
                                                continue_to_work = 0;
                                        }else printf("%s\n", message);

                                        // EPOLLIN event from child process(user's input message)
                                }else{
                                        if(DEBUG_MODE) printf("New pipe event!\n");
                                        CHK2(res, read(events[i].data.fd, message, BUF_SIZE));

                                        // zero size of result means the child process going to exit
                                        if(res == 0) continue_to_work = 0; // exit parent to
                                        // send message to server
                                        else{
                                                CHK(send(sock, message, BUF_SIZE, 0));
                                        }
                                }
                        }
                }
}
if(pid){
        if(DEBUG_MODE) printf("Shutting down parent!\n");
        close(pipe_fd[0]);
        close(sock);
}else{
        if(DEBUG_MODE) printf("Shutting down child!\n");
        close(pipe_fd[1]);
}

return 0;
}