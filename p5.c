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

#define NON_RESERVED_PORT 5001
#define BUFFER_LEN 1024
#define SERVER_ACK "ack from server \n"
// function prototypes
void server(void);
int setup_to_accept(int por);
int accept_connection(int accept_socket);
void serve_one_connection(int client_socket);
void client(char * server_host);
int connect_to_server(char * hostname, int port);
void send_msg(int fd, char * buf, int size);
int recv_msg(int fd, char *buf);
void error_check(int val, const char * str);

// main
int main(int argc, char *argv[]) {

  // if two args then running as client
  if(argc == 2){
    printf("calling the client\n" );
    client(argv[1]);
  }
  else // if no args then running as server
  {
    printf("Running the Server on port %d. \n", NON_RESERVED_PORT);
    server();
    printf("ack back from the server \n" );
  }
  return 0;
}

void server()
{
  int rc, accept_socket, client_socket;
  accept_socket = setup_to_accept(NON_RESERVED_PORT);
  for(;;)
  {
      client_socket = accept_connection(accept_socket);
      serve_one_connection(client_socket);
  }
}

void serve_one_connection(int client_socket)
{
    int rc, ack_length;
    char buf[BUFFER_LEN];
    ack_length = strlen(SERVER_ACK) + 1;
    rc = recv_msg(client_socket, buf);
    buf[rc] = '\0';
    while (rc != 0) {
      printf("Server recieved %d bytes, string: %s\n",rc, buf );
      send_msg(client_socket, (char*)SERVER_ACK, ack_length);
      rc = recv_msg(client_socket, buf);
      buf[rc] = '\0';
    }
    close(client_socket);
}


int accept_connection(int accept_socket){
  struct sockaddr_in from;
  int fromlen, client_socket, gotit;

  fromlen = sizeof(from);
  gotit = 0;
  while(!gotit)
  {
    client_socket = accept(accept_socket,(struct sockaddr *)&from, (socklen_t*)&fromlen);
    if(client_socket == -1)
    {
      // if interrupted then try again:
      if(errno == EINTR )
      {
        continue;
      }
      else
      {
          error_check(client_socket, "accept connection functions");
      }
    }
    else{
      gotit = 1;
    }
  } // end WHILE
}// end accept_connection

// client takes in an address ipv4.
void client(char * server_host){
  int rc, client_socket;
  char buffer[BUFFER_LEN];
  client_socket = connect_to_server(server_host, NON_RESERVED_PORT);
}


int connect_to_server(char * hostname, int port){
  int client_socket;
  /*
  struct sockaddr_in {
    sa_family_t    sin_family; /// address family: AF_INET /
    in_port_t      sin_port;   /// port in network byte order /
    struct in_addr sin_addr;   /// internet address /
    };

  // Internet address.
    struct in_addr {
    uint32_t       s_addr;     ///// address in network byte order /
    };

*/

  struct sockaddr_in listener;
    /*
    typedef struct hostent {
      char  *h_name; host pc name or DNS
      char  **h_aliases;
      short h_addrtype; type of address being returned
      short h_length;
      char  **h_addr_list; length in bytes of each address
    } HOSTENT, *PHOSTENT, *LPHOSTENT;
    */
  struct hostent *hp;
  int rc;
  int optval = 1;
  // results of a successful search of the name parameter, ipv4 here I think
  hp = gethostbyname(hostname);

  if(hp == NULL)
  {
    printf("connecting to server: %s failed. Exiting %s\n",hostname, strerror(errno) );
    exit(-1);
  }
  // zero out listener struct
  bzero((void *)&listener, sizeof(listener));
  //ByteCopy: (source, dest, size). Deprecated, use memcopy
  // h_addr is the first address. copy to listener.sin_addr
  bcopy( (void*)hp->h_addr, (void*)&listener.sin_addr, hp->h_length  );

  listener.sin_family = hp->h_addrtype;
  // convert port to network byte order
  listener.sin_port = htons(port);
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  rc = connect(client_socket, (struct sockaddr *)&listener, sizeof(listener));
  setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval,sizeof(optval));
  return client_socket;
}

int setup_to_accept(int port){
  int rc, accept_socket;
  int optval = 1;
  struct sockaddr_in sin;

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  // create a socket to accept traffic:
  accept_socket = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(accept_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
}

int recv_msg(int fd, char * buf)
{
  int bytes_read;
  // read takes a fild descriptor, a buffer to place it message, length of message
  // number of bytes read is returned and the pointer is incremented by that number of bytes
  bytes_read = read(fd, buf, BUFFER_LEN );
  return bytes_read;
}

void send_msg(int fd, char * buf, int size ){
  int n;
  n = write(fd, buf, size);
}

void error_check(int val, const char *str)
{
    if (val < 0)
    {
        printf("%s :%d: %s\n", str, val, strerror(errno));
        exit(1);
    }
}
