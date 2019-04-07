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

#define NON_RESERVED_PORT    6200
#define BUFFER_LEN           1024
#define SERVER_ACK           "ACK_FROM_SERVER\n"

void client();
int connect_to_server(int port);
int recv_msg(int fd, char *buf);
void send_msg(int fd, char *buf, int size);
void error_check(int val, const char *str);

int main(int argc, char *argv[]) {
  client();
  return 0;
}

void client(){
    int rc, client_socket;
    char buf[BUFFER_LEN];
    client_socket = connect_to_server(NON_RESERVED_PORT);
    printf("\n Enter a line of text to send to the server or EOF to exit\n");
    while (fgets(buf,BUFFER_LEN,stdin) != NULL)
    {
        send_msg(client_socket, buf, strlen(buf)+1);
        rc = recv_msg(client_socket, buf);
        buf[rc] = '\0';
        printf("client received %d bytes  :%s: \n",rc,buf);
        printf("\nEnter a line of text to send to the server or EOF to exit\n");
    }
}

int connect_to_server(int port){
    int client_socket;
    struct sockaddr_in listener;
    struct hostent *hp;
    int rc;
    int optval = 1;
    hp = gethostbyname("localhost");
    bzero((void *)&listener, sizeof(listener));
    bcopy((void *)hp->h_addr, (void *)&listener.sin_addr, hp->h_length);
    listener.sin_family = hp->h_addrtype;
    // Create socket //
    listener.sin_port = htons(port);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    error_check(client_socket, "net_connect_to_server socket");
    // Connect //
    rc = connect(client_socket,(struct sockaddr *) &listener, sizeof(listener));

    error_check(rc, "net_connect_to_server connect");
    setsockopt(client_socket,IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval));
    return(client_socket);
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
  if (val < 0) {
        printf("%s :%d: %s\n", str, val, strerror(errno));
        exit(1);
    }
}
