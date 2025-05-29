#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "utils.h"
#include "market_data.h"

typedef struct
{
    int size;
    ohlc_t data[];
} candle_shmap_t;


void* get_agg(const char* agg_name)
{
    int fd = open(agg_name, O_RDONLY, 0400);
    void* agg = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    return agg;
}


candle_shmap_t* candles = NULL;
int window_start = 0;

void init()
{
    candles = get_agg("candles_AAA_5M");
}

void on_trade()
{
    if (candles->size < 3)
        return;
    
    outl("three candles breakout EA boiiiii");
    ohlc_t c1 = candles->data[candles->size - 1];
    ohlc_t c2 = candles->data[candles->size - 2];
    ohlc_t c3 = candles->data[candles->size - 3];

    outl("c1 %lf %lf %lf %lf", c1.open, c1.high, c1.low, c1.close);
    outl("c2 %lf %lf %lf %lf", c2.open, c2.high, c2.low, c2.close);
    outl("c3 %lf %lf %lf %lf", c3.open, c3.high, c3.low, c3.close);

    double max_c2_c3_hi = c2.high >= c3.high ? c2.high : c3.high;
    double min_c2_c3_lo = c2.low <= c3.low ? c2.low : c3.low;
    
    int buy_condition = 
        c3.close < c3.open &&
        c2.close < c2.open &&
        c1.close > c1.open &&
        c1.close > max_c2_c3_hi;
        
    int sell_condition = 
        c3.close > c3.open &&
        c2.close > c2.open &&
        c1.close < c1.open &&
        c1.close < min_c2_c3_lo;

    if (buy_condition)
        outl("gonna buy!!");
    if (sell_condition)
        outl("gonna sell!");
}


int main()
{
    init();
    on_trade();
    return 0;
}

