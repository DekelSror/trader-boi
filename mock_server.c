#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "mock_server.h"
#include "market_data.h"

int mock_server_init(void) {
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = SOCKET_PATH,
    };      

    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    unlink(server_addr.sun_path);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Market data server started on %s. Waiting for clients...\n", SOCKET_PATH);
    
    return server_sock;
}

int mock_server_accept_client(int server_sock) {
    int client_sock;
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Client connected. Starting data feed...\n");
    
    return client_sock;
}

ssize_t mock_server_send_message(int client_sock, parsed_message_t *msg) {
    ssize_t bytes_sent = send(client_sock, msg, sizeof(*msg), 0);
    if (bytes_sent < 0) {
        perror("send failed for message");
    }
    return bytes_sent;
}

void mock_server_cleanup(int server_sock, int client_sock) {
    if (client_sock >= 0) {
        close(client_sock);
    }
    
    if (server_sock >= 0) {
        close(server_sock);
    }
    
    unlink(SOCKET_PATH);
    printf("Server cleaned up.\n");
} 