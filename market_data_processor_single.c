#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>

#include "NxCoreAPI.h"
#include "nx_parsers.h"
#include "market_data.h"
#include "alg.h"

#define MQ_NAME "/parsed_market_data"
#define MQ_MAX_MESSAGES 10
#define MQ_MAX_MSG_SIZE sizeof(parsed_message_t)

static NxCoreProcessTape market_callback = NULL;
static size_t message_count = 0;
static mqd_t mq = -1;

void print_trade(const trade_t* trade) {
    if (!trade) return;
    
    time_t seconds = trade->timestamp / 1000000;
    int microseconds = trade->timestamp % 1000000;
    struct tm* tm_info = localtime(&seconds);
    
    printf("TRADE: %04d-%02d-%02d %02d:%02d:%02d.%06d | Symbol: %s | Price: %.2f | Size: %d | Side: %c (%d) | Condition: %s\n",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds,
           trade->symbol, trade->price, trade->volume, trade->side, (int)trade->side, trade->condition);
}

void print_depth(const market_depth_t* depth) {
    if (!depth) return;
    
    time_t seconds = depth->timestamp / 1000000;
    int microseconds = depth->timestamp % 1000000;
    struct tm* tm_info = localtime(&seconds);
    
    printf("\nMARKET DEPTH: %04d-%02d-%02d %02d:%02d:%02d.%06d | Symbol: %s | Spread: %.2f\n",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds,
           depth->symbol, depth->spread);
    
    printf("BIDS:\n");
    for (int i = 0; i < 10; i++) {
        if (depth->bids[i].size > 0) {
            printf("  %.2f x %d (%d orders)\n",
                   depth->bids[i].price,
                   depth->bids[i].size,
                   depth->bids[i].orders);
        }
    }
    
    printf("ASKS:\n");
    for (int i = 0; i < 10; i++) {
        if (depth->asks[i].size > 0) {
            printf("  %.2f x %d (%d orders)\n",
                   depth->asks[i].price,
                   depth->asks[i].size,
                   depth->asks[i].orders);
        }
    }
    printf("\n");
}

int __stdcall parse_message(const struct NxCoreSystem* sys, const struct NxCoreMessage* msg) 
{
    parsed_message_t pm = {0};
    
    if (msg->MessageType == NxMSG_TRADE)
    {
        pm.type = MSG_TRADE;
        nxcore_to_trade(msg, &msg->coreData.Trade, &pm.event.trade);
        print_trade(&pm.event.trade);
        
    }
    else if (msg->MessageType == NxMSG_EXGQUOTE)
    {
        pm.type = MSG_DEPTH;
        nxcore_to_market_depth(msg, &msg->coreData.ExgQuote, &pm.event.depth);
        print_depth(&pm.event.depth);
    }
    else
    {
        printf("unknown event type %d\n", msg->MessageType);
        return NxCALLBACKRETURN_CONTINUE;   
    }
    

    if (mq_send(mq, (char*)&pm, sizeof(pm), 0) == -1) {
        fprintf(stderr, "Failed to send depth to queue: %s\n", strerror(errno));
    }

    sleep(1);
    return NxCALLBACKRETURN_CONTINUE;
}

void print_nx_datetime(NxDate* d, NxTime* t)
{
    printf("%04d-%02d-%02d:%02d:%02d:%02d",
        d->Year, d->Month, d->Day,
        t->Hour, t->Minute, t->Second
    );
}

int main()
{
    const char* data_path = "/home/dekel/Downloads/20210601.TV.nx2";
    int test_fd = open(data_path, O_RDONLY);

    close(test_fd);

    struct mq_attr attr;
    attr.mq_flags = 0;                // Flags (0 = blocking)
    attr.mq_maxmsg = MQ_MAX_MESSAGES; // Max number of messages in queue
    attr.mq_msgsize = MQ_MAX_MSG_SIZE;// Max message size
    attr.mq_curmsgs = 0;              // Current number of messages

    mq_unlink(MQ_NAME);

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        fprintf(stderr, "Failed to create message queue: %s\n", strerror(errno));
        return 1;
    }

    void* libnx = dlopen("./libnx.so", RTLD_LAZY);
    if (!libnx)
    {
        fprintf(stderr, "Failed to load libnx.so: %s\n", dlerror());
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return 1;
    }

    market_callback = (NxCoreProcessTape)dlsym(libnx, cszNxCoreProcessTape);
    if (!market_callback)
    {
        fprintf(stderr, "Failed to get NxCoreProcessTape function: %s\n", dlerror());
        dlclose(libnx);
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return 1;
    }

    int result = market_callback(data_path, NULL, 0, 0, parse_message);
    if (result != NxAPIERR_NO_ERROR)
    {
        fprintf(stderr, "Error processing NxCore data: %d\n", result);
        dlclose(libnx);
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return 1;
    }

    dlclose(libnx);

    // Close and unlink the message queue (optional - can also be kept open for reconnection)
    mq_close(mq);
    // mq_unlink(MQ_NAME); // Uncomment to remove the queue when done

    return 0;
} 