#include <stdlib.h>
#include <string.h>

#include "candle_agg.h"


candle_agg_t agg_init(long freq, int(*post)(ohlc_t))
{
    candle_agg_t agg = {
        .post_candle = post,
        .frequency = freq,
        .ongoing = 0,
    };

    return agg;
}

int agg_on_trade(candle_agg_t* agg, trade_t trade)
{
    if (!agg->ongoing)
    {
        agg->candle.close = trade.price;
        agg->candle.open = trade.price;
        agg->candle.high = trade.price;
        agg->candle.low = trade.price;
        agg->candle.timestamp = trade.timestamp;
        memmove(agg->candle.symbol, trade.symbol, 4);

        agg->ongoing = 1;
    }

    if (trade.price > agg->candle.high)
    {
        agg->candle.high = trade.price;
    }

    if (trade.price < agg->candle.low)
    {
        agg->candle.low = trade.price;
    }

    if (trade.timestamp - agg->candle.timestamp >= agg->frequency)
    {
        agg->candle.close = trade.price;
        agg->post_candle(agg->candle);
        memset(&agg->candle, 0, sizeof(agg->candle));
        agg->ongoing = 0;
    }

    return 0;
}
