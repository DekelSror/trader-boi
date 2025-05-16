#include <time.h>
#include <stdio.h>

#include "print_utils.h"


void print_trade(const trade_t* trade)
{
    if (!trade) return;
    
    time_t seconds = trade->timestamp / 1000000;
    int microseconds = trade->timestamp % 1000000;
    struct tm* tm_info = localtime(&seconds);
    
    printf("TRADE: %04d-%02d-%02d %02d:%02d:%02d.%06d | Symbol: %s | Price: %.2f | Size: %d | Side: %c (%d) | Condition: %s\n",
        tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds,
        trade->symbol, trade->price, trade->volume, trade->side, (int)trade->side, trade->condition
    );
}

void print_depth(const market_depth_t* depth)
{
    if (!depth) return;
    
    time_t seconds = depth->timestamp / 1000000;
    int microseconds = depth->timestamp % 1000000;
    struct tm* tm_info = localtime(&seconds);
    
    printf("\nMARKET DEPTH: %04d-%02d-%02d %02d:%02d:%02d.%06d | Symbol: %s | Spread: %.2f\n",
        tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds,
        depth->symbol, depth->spread
    );
    
    printf("BIDS:\n");
    for (int i = 0; i < 10; i++)
    {
        if (depth->bids[i].size > 0)
        {
            printf("  %.2f x %d (%d orders)\n",
                depth->bids[i].price,
                depth->bids[i].size,
                depth->bids[i].orders
            );
        }
    }
    
    printf("ASKS:\n");
    for (int i = 0; i < 10; i++)
    {
        if (depth->asks[i].size > 0)
        {
            printf("  %.2f x %d (%d orders)\n",
                depth->asks[i].price,
                depth->asks[i].size,
                depth->asks[i].orders
            );
        }
    }
    printf("\n");
}