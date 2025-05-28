#ifndef candle_agg_h
#define candle_agg_h

#include "market_data.h"


typedef struct
{
    long frequency;
    int(*post_candle)(ohlc_t);
    int ongoing;
    ohlc_t candle;
} candle_agg_t;


candle_agg_t agg_init(long freq, int(*post)(ohlc_t));
int agg_on_trade(candle_agg_t* agg, trade_t trade);


#endif // candle_agg_h