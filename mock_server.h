#ifndef MOCK_SERVER_H
#define MOCK_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "market_data.h" // For parsed_message_t

#define SOCKET_PATH "/tmp/trader-boi-comm"

/**
 * Initialize the mock server
 * 
 * @return The server socket file descriptor
 */
int mock_server_init(void);

/**
 * Accept a client connection
 * 
 * @param server_sock The server socket
 * @return The client socket file descriptor
 */
int mock_server_accept_client(int server_sock);

/**
 * Send a message to the client
 * 
 * @param client_sock The client socket
 * @param msg The message to send
 * @return Number of bytes sent, or -1 on error
 */
ssize_t mock_server_send_message(int client_sock, parsed_message_t *msg);

/**
 * Clean up server resources
 * 
 * @param server_sock The server socket
 * @param client_sock The client socket
 */
void mock_server_cleanup(int server_sock, int client_sock);

#endif /* MOCK_SERVER_H */ 