#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "market_data.h"
#include "utils.h"

double balance = 1000.0;

void alg1_init()
{
    printf("alg1 up and running\n");
}


void alg1_on_trade(trade_t* trade)
{
    if (!trade) return;
    printf("ALG1 processing trade for %s at price %.2f\n", trade->symbol, trade->price);

    if (strneq(trade->symbol, "AAA", 4))
    {
        if (trade->price >= 150.0)
        {
            // send a bid for AAA, volume 1, price 
        }
    }
}

void alg1_on_depth(market_depth_t* depth)
{
    if (!depth) return;
    printf("ALG1 processing depth for %s with spread %.2f\n", depth->symbol, depth->spread);
}

void(*alg1_handlers[])(void*) = {
    alg1_on_trade,
    alg1_on_depth,
};

int load_algs()
{
    printf("Loading algorithms...\n");
    alg1_init();
    return 0;
}

int main()
{
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = "/tmp/trader-boi-comm",
    };

    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
        return 1;
    }

    unlink(server_addr.sun_path);

    if (bind(sock, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Failed to bind socket: %s (path: %s)\n", strerror(errno), server_addr.sun_path);
        close(sock);
        return 1;
    }

    printf("Strategy processor started. Press Ctrl+C to exit.\n");

    load_algs();

    parsed_message_t pm;
    ssize_t event_size;
    while (1)
    {
        event_size = recvfrom(sock, &pm, sizeof(pm), 0, NULL, NULL);
        if (event_size == -1)
        {
            fprintf(stderr, "Error receiving message: %s\n", strerror(errno));
            break;
        }
        
        if (event_size >= sizeof(pm))
        {
            if (pm.type < 2)
            {
                // send event to aggregators and the such
                alg1_handlers[pm.type](&pm.event);
            }
            else
            {
                printf("got an unknown event type %d\n", pm.type);
            }
        }
    }

    close(sock);
    printf("Strategy processor shut down\n");
    
    return 0;
}

