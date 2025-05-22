#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "socket_provider.h"

#define SOCKET_PATH "/tmp/trader-boi-comm"

int sock = -1;

static int socket_provider_connect()
{
    struct sockaddr_un server_addr = {
        .sun_family = AF_UNIX,
        .sun_path = SOCKET_PATH,
    };

    // Create a TCP socket
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
        return 1;
    }

    printf("Connecting to market data server...\n");

    // Connect to the market data server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Failed to connect to market data server: %s\n", strerror(errno));
        fprintf(stderr, "Make sure the market data server (mocker) is running first.\n");
        close(sock);
        return 1;
    }
}

static int get_msg(parsed_message_t* buf)
{
    return (int)recv(sock, buf, sizeof(*buf), 0);
}

static int disconnect()
{
    close(sock);
    sock = -1;
}

const remote_provider_api_t SocketProviderAPI = {
    .connect = socket_provider_connect,
    .get_msg = get_msg,
    .disconnect = disconnect,
};

