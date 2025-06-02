#ifndef candle_agg_h
#define candle_agg_h

#include "market_data.h"


typedef struct
{
    long frequency;
    char symbol[8];
    int(*post_candle)(ohlc_t);
    int ongoing;
    ohlc_t candle;
} candle_agg_t;


candle_agg_t candle_agg_init(const char* symbol, long freq, int(*post)(ohlc_t));
void candle_agg_on_trade(candle_agg_t* agg, trade_t trade);


#endif // candle_agg_h