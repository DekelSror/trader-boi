#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>

#include "market_data.h"
#include "utils.h"

#define MQ_NAME "/parsed_market_data"
#define MQ_MAX_MSG_SIZE sizeof(parsed_message_t)

static mqd_t mq = -1;

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
    mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        fprintf(stderr, "Failed to open message queue: %s\n", strerror(errno));
        fprintf(stderr, "Make sure market_data_processor is running first\n");
        return 1;
    }

    printf("Strategy processor started. Press Ctrl+C to exit.\n");

    load_algs();

    parsed_message_t pm;
    unsigned int prio;
    ssize_t event_size;

    while (1)
    {
        event_size = mq_receive(mq, (char*)&pm, MQ_MAX_MSG_SIZE, &prio);

        if (event_size == -1)
        {
            fprintf(stderr, "Error receiving from queue: %s\n", strerror(errno));
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

    mq_close(mq);
    printf("Strategy processor shut down\n");
    
    return 0;
}

