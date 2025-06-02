#include <string.h>

#include "utils.h"
#include "candle_agg.h"


/*
TODO:
    handle empty candles - 
    * intervals where no trades occur
    * first interval - if no trades, get values from somewhere
    * either prev day or what have you
*/


candle_agg_t candle_agg_init(const char* symbol, long freq, int(*post)(ohlc_t))
{
    candle_agg_t agg = {
        .post_candle = post,
        .frequency = freq,
        .ongoing = 0,
    };

    memmove(agg.symbol, symbol, 8);

    return agg;
}

void candle_agg_on_trade(candle_agg_t* agg, trade_t trade)
{
    if (!strneq(agg->symbol, trade.symbol, 8))
        return;

    if (!agg->ongoing)
    {
        agg->candle.close = trade.price;
        agg->candle.open = trade.price;
        agg->candle.high = trade.price;
        agg->candle.low = trade.price;
        agg->candle.timestamp = trade.timestamp;
        memmove(agg->candle.symbol, agg->symbol, 8);

        agg->ongoing = 1;
        return;
    }

    if (trade.price > agg->candle.high)
        agg->candle.high = trade.price;

    if (trade.price < agg->candle.low)
        agg->candle.low = trade.price;

    if (trade.timestamp - agg->candle.timestamp >= agg->frequency)
    {
        agg->candle.close = trade.price;
        agg->post_candle(agg->candle);
        outl("made a candle for %s", agg->symbol);
        memset(&agg->candle, 0, sizeof(agg->candle));
        agg->ongoing = 0;
    }

    return 0;
}
