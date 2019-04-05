#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#define BUFFER_LEN           1024
#define SERVER_ACK           "ACK_FROM_SERVER\n"

void server(void);
int setup_to_accept(int por);
int accept_connection(int accept_socket);
void serve_one_connection(int client_socket);
///////////////////////////////////////////
void send_msg(int fd, char *buf, int size);
int recv_msg(int fd, char *buf);
void error_check(int val, const char *str);

int NON_RESERVED_PORT;

int main(int argc, char const *argv[]) {
  NON_RESERVED_PORT = atoi(argv[1]);
  server();
  return 0;
}

void server(){
    int rc, accept_socket, client_socket;
    accept_socket = setup_to_accept(NON_RESERVED_PORT);
    for (;;){
      client_socket = accept_connection(accept_socket);
      serve_one_connection(client_socket);
    }
}

int setup_to_accept(int port){
    int rc, accept_socket;
    int optval = 1;
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    accept_socket = socket(AF_INET, SOCK_STREAM, 0);
    error_check(accept_socket, "setup_to_accept socket");
    setsockopt(accept_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval));
    rc = bind(accept_socket, (struct sockaddr *)&sin ,sizeof(sin));
    error_check(rc, "setup_to_accept bind");
    rc = listen(accept_socket, 5);
    error_check(rc, "setup_to_accept listen");
    return(accept_socket);
}

int accept_connection(int accept_socket){
    struct sockaddr_in from;
    int fromlen, client_socket, gotit;
    int optval = 1;
    fromlen = sizeof(from);
    gotit = 0;
    while (!gotit)
    {
        client_socket = accept(accept_socket, (struct sockaddr *)&from,
                               (socklen_t *)&fromlen);
        if (client_socket == -1)
        {
            /* Did we get interrupted? If so, try again */
            if (errno == EINTR)
                continue;
            else
                error_check(client_socket, "accept_connection accept");
        }
        else{ gotit = 1;}
    }
    setsockopt(client_socket,IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval));
    return(client_socket);
}

void serve_one_connection(int client_socket){
    int rc, ack_length;
    char buf[BUFFER_LEN];
    ack_length = strlen(SERVER_ACK)+1;
    rc = recv_msg(client_socket, buf);
    buf[rc] = '\0';
    while (rc != 0)
    {
        printf("server received %d bytes  :%s: \n",rc,buf);
        send_msg(client_socket, (char *)SERVER_ACK, ack_length);
        rc = recv_msg(client_socket, buf);
        buf[rc] = '\0';
    }
    close(client_socket);
}

int recv_msg(int fd, char *buf){
    int bytes_read;
    bytes_read = read(fd, buf, BUFFER_LEN);
    error_check( bytes_read, "recv_msg read");
    return( bytes_read );
}

void send_msg(int fd, char *buf, int size){
    int n;
    n = write(fd, buf, size);
    error_check(n, "send_msg write");
}

void error_check(int val, const char *str){
    if (val < 0){
        printf("%s :%d: %s\n", str, val, strerror(errno));
        exit(1);
    }
}
