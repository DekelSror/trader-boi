#include <time.h>

#include "socket_provider.h"
#include "remote_provider.h"
#include "timeseries.h"
#include "shmap_ts.h"
#include "utils.h"
#include "candle_agg.h"

remote_provider_api_t provider;
timeseries_api_t tsdb;
ts_iterator_api_t ts_iter;


ohlc_t aaa_5m_candles[128] = {0};
int num_aaa_5m_candles = 0;

int post_aaa_5m_candle(ohlc_t entry)
{
    if (num_aaa_5m_candles == 128)
    {
        return 1;
    }

    memmove(aaa_5m_candles + num_aaa_5m_candles, &entry, sizeof(entry));
    num_aaa_5m_candles++;
    return 0;
}

int main()
{
    provider = SocketProviderAPI;
    tsdb = ShMapTimeseries;
    ts_iter = ShMapTsIterator;
    candle_agg_t candle_agg = agg_init(1000000 * 60 * 5, post_aaa_5m_candle);

    tsdb.create("AAA");
    tsdb.create("BBB");

    provider.connect();

    parsed_message_t msg;
    while (1)
    {
        int msglen = provider.get_msg(&msg);

        if (msglen <= 0)
        {
            break;
        }

        if (msg.type == MSG_TRADE)
        {
            trade_t trade = msg.event.trade;

            int err = tsdb.add(trade.symbol, trade.timestamp, trade.price);
            outl("got %s %ld %lf %d", trade.symbol, trade.timestamp, trade.price, err);
            agg_on_trade(&candle_agg, trade);
        }
    }

    void* iter = tsdb.frame_by_index("AAA", 0, -1);

    while (!ts_iter.is_spent(iter))
    {
        outl("%lf", ts_iter.next(iter).v);
    }

    provider.disconnect();

    for (int i = 0; i < num_aaa_5m_candles; i++)
    {
        ohlc_t candle = aaa_5m_candles[i];
        time_t seconds = candle.timestamp / 1000000;
        int microseconds = candle.timestamp % 1000000;
        struct tm* tm_info = localtime(&seconds);
        
        outl("candle %d: timestamp %04d-%02d-%02d %02d:%02d:%02d.%06d | Symbol: %s | open %02lf close %02lf high %02lf low %02lf",
            i,
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds,
            candle.symbol, candle.open, candle.close, candle.high, candle.low
        );
    }
    

    return 0;
}
