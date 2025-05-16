#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "alg.h"

#define MQ_NAME "/parsed_market_data"
#define MQ_MAX_MSG_SIZE sizeof(parsed_message_t)

static mqd_t mq = -1;
static volatile sig_atomic_t running = 1;

void alg1_init()
{
    printf("alg1 up and running\n");
}

void alg1_on_trade(trade_t* trade)
{
    if (!trade) return;
    printf("ALG1 processing trade for %s at price %.2f\n", trade->symbol, trade->price);
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

int main(int argc, char const *argv[], char** envp)
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
    ssize_t bytes_read;
    
    while (running)
    {
        bytes_read = mq_receive(mq, (char*)&pm, MQ_MAX_MSG_SIZE, &prio);
        
        if (bytes_read == -1)
        {
            fprintf(stderr, "Error receiving from queue: %s\n", strerror(errno));
            break;
        }
        
        if (bytes_read >= sizeof(pm))
        {
            alg1_handlers[pm.type](&pm);
            if (pm.type == MSG_TRADE)
            {
                alg1_on_trade(&pm.event.trade);
            } 
            else if (pm.type == MSG_DEPTH)
            {
                alg1_on_depth(&pm.event.depth);
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

