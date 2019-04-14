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
#include <stdbool.h>

#define BUFFER_LEN 256 
#define SERVER_ACK  "Msg acknowledged by Server\n"
#define maxStruct 64 

typedef struct keyvalue{
  bool flag;
  char key[32];
  char value[32];
}keyvalue;
struct keyvalue kv[maxStruct];

void put_keyvalue(char * buf);
void get_value(char * buf);
void server(void);
int setup_to_accept(int por);
int accept_connection(int accept_socket);
void serve_one_connection(int client_socket);
void send_msg(int fd, char *buf, int size);
int recv_msg(int fd, char *buf);

int NON_RESERVED_PORT;

int main(int argc, char const *argv[]) {
  // set up structs //
  for(int i = 0; i < maxStruct; i++)
  {
    kv[i].flag = false;
    memset(kv[i].key, '\0', sizeof(kv[i].key));
    memset(kv[i].value, '\0', sizeof(kv[i].value));
  }
  if (argc < 2) { printf("enter port number \n" ); exit(0); }
  if(strcmp(argv[1], "-hw") == 0) { printf("%s\n","hello world" ); exit(0); }
  NON_RESERVED_PORT = atoi(argv[1]);
  server();
  return 0;
}

void server(){
    int rc, accept_socket, client_socket;
    accept_socket = setup_to_accept(NON_RESERVED_PORT);
    // While true //
    for (;;){
      printf("server is listening\n" );
      client_socket = accept_connection(accept_socket);
      serve_one_connection(client_socket);
    }
}

int setup_to_accept(int port){
    //printf("setup_to_accept called\n" );
    int rc, accept_socket;
    int optval = 1;
    // sock address struct
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    // create socket //
    accept_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(accept_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval));
    // bind socket
    rc = bind(accept_socket, (struct sockaddr *)&sin ,sizeof(sin));

    // listen //               num of connections to handle at a time.
    rc = listen(accept_socket, 5);
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
              continue;
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
        send_msg(client_socket, (char *)SERVER_ACK, ack_length);
        rc = recv_msg(client_socket, buf);
        buf[rc] = '\0';
    }
    close(client_socket);
}

int recv_msg(int fd, char *buf){
    int bytes_read;
    bytes_read = read(fd, buf, BUFFER_LEN);
    if (bytes_read != 0){
      // put value
      if (strstr(buf, "put")) {
        put_keyvalue(buf);
      }else if(strstr(buf, "get")){
        get_value(buf);
      } else {
        return(bytes_read);
      }
    }
    return( bytes_read );
}

void put_keyvalue(char * buf){
    printf("%s\n",buf);
    // store value:
    for(int i = 0; i < maxStruct; i++){
      if(kv[i].flag == false){
        char * token = strtok(buf, " ");
        token = strtok(NULL, " ");
        if(token != NULL) { strcpy(kv[i].key, token); }
        token = strtok(NULL, " ");
        if(token != NULL) {strcpy(kv[i].value, token); }
        kv[i].flag = true;
        memset(buf, '\0',BUFFER_LEN);
        break;
        }
      }
}

void get_value(char * buf){
    char * token = strtok(buf, " ");
    token = strtok(NULL, " ");
    printf("get %s\n", token);
    for (size_t j = 0; j < maxStruct; j++) {
      if( (strstr(token, kv[j].key)) && (token != NULL) ){
        printf("%s %s\n", kv[j].key, kv[j].value);
        memset(buf, '\0',BUFFER_LEN);
        break;
      }
    }
}

void send_msg(int fd, char *buf, int size){
    int n;
    n = write(fd, buf, size);
}
