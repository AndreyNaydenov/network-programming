#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#define PORT "3322"

void print_client_info(struct sockaddr *client_addr) {
  char host[NI_MAXHOST], service[NI_MAXSERV];
  int result = getnameinfo(client_addr, client_addr->sa_len, host, sizeof(host), service,
              sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
  if (result == 0) {
    printf("Accepted connection from %s:%s\n", host, service);
  } else {
    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(result));
  }
  printf("Client info: %s:%s\n", host, service);
}

int main(int argc, char **argv) {
  // get local address info
  // listen on port 3322
  // when someone connects, send them hello message and the listen for their
  // answer

  struct sockaddr_storage client_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd, clientfd;

  // init hints with zeros
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;

  // get local address and port
  int err = getaddrinfo(NULL, PORT, &hints, &res);
  if (err != 0) {
    printf("Got error while getting address info: %d\n", err);
  }

  // print address that we got from getaddrinfo
  char ipstr[INET6_ADDRSTRLEN];
  char *ipver;
  if (res->ai_family == AF_INET) {
    inet_ntop(res->ai_family,
              &((struct sockaddr_in const *)res->ai_addr)->sin_addr, ipstr,
              INET6_ADDRSTRLEN);
    ipver = "IPv4";
  } else {
    inet_ntop(res->ai_family,
              &((struct sockaddr_in6 const *)res->ai_addr)->sin6_addr, ipstr,
              INET6_ADDRSTRLEN);
    ipver = "IPv6";
  }
  printf("Will listen on: %s %s:%s\n", ipver, ipstr, PORT);

  // socket and build syscalls
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  err = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (err != 0) {
    printf("Got error while binding to address %s: %d\n", ipstr, err);
    return 1;
  }

  // start to listen
  printf("Starting to listen...\n");
  err = listen(sockfd, 10);
  if (err != 0) {
    printf("Got error while starting to listen: %d\n", err);
  }

  // accepting the connection
  // TODO: Fix some kind of bug that I don't get info about client (sockaddr is not populating)
  clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
  if (clientfd == -1) {
    perror("Got error while accepting connection from client");
  }
  
  print_client_info((struct sockaddr *)&client_addr);
  printf("new_fd %d socket_fd %d\n", clientfd, sockfd);

  // send hello
  char *msg = "Hi, my new client!\n";
  int bytes_sent, len;
  len = strlen(msg);
  bytes_sent = send(clientfd, msg, len, 0);
  if (bytes_sent == -1) {
    perror("Failed to send");
    return 1;
  }

  // listen to answer and print it
  int bytes_received;
  char buf[50];
  len = 50;
  bytes_received = recv(clientfd, buf, len, 0);
  if (bytes_received == -1) {
    perror("Failed to receive");
    return 1;
  }
  if (bytes_received == 0) {
    printf("Client closed the connection\n");
    return 1;
  }
  // print the message
  printf("Message received: %s\n", buf);

  freeaddrinfo(res);
  return 0;
}
