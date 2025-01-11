#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "8080"  // Port number as a string
#define BACKLOG 10   // Number of pending connections queue will hold

void print_client_info(struct sockaddr *client_addr) {
    char host[NI_MAXHOST], service[NI_MAXSERV];
    int result = getnameinfo(client_addr, client_addr->sa_len, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
    if (result == 0) {
        printf("Accepted connection from %s:%s\n", host, service);
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(result));
    }
}

int main() {
    struct addrinfo hints, *res, *p;
    int server_fd, client_fd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Initialize hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    // For wildcard IP address

    // Get address information
    int status = getaddrinfo(NULL, PORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Loop through all the results and bind to the first we can
    for (p = res; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd == -1) {
            perror("socket");
            continue;
        }

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_fd);
            perror("bind");
            continue;
        }

        break;  // Successfully bound
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);  // Free the linked list

    // Listen for incoming connections
    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %s\n", PORT);

    // Accept a connection
    while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) != -1) {
        print_client_info((struct sockaddr *)&client_addr);

        // Close the client socket
        close(client_fd);
    }

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Close the server socket
    close(server_fd);

    return 0;
}
