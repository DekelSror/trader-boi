#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "socket_provider.h"
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
    int sock = SocketProviderAPI.connect();

    printf("Connected to market data server. Starting algorithm...\n");

    load_algs();

    parsed_message_t pm;
    while (1)
    {
        int event_size = SocketProviderAPI.get_msg(&pm);
        if (event_size == -1)
        {
            fprintf(stderr, "Error receiving message: %s\n", strerror(errno));
            break;
        }
        
        // If we get 0 bytes, the server has closed the connection
        if (event_size == 0) {
            printf("Market data server disconnected.\n");
            break;
        }
        
        if (event_size >= sizeof(pm))
        {
            if (pm.type < 2)
            {
                // Process the market data with our algorithms
                alg1_handlers[pm.type](&pm.event);
            }
            else
            {
                printf("got an unknown event type %d\n", pm.type);
            }
        }
    }

    SocketProviderAPI.disconnect();
    
    return 0;
}

