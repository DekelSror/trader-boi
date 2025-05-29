#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "socket_provider.h"
#include "remote_provider.h"
#include "timeseries.h"
#include "shmap_ts.h"
#include "utils.h"
#include "candle_agg.h"

remote_provider_api_t provider;
timeseries_api_t tsdb;
ts_iterator_api_t ts_iter;


typedef struct
{
    int size;
    ohlc_t data[];
} candle_shmap_t;



candle_shmap_t* candles = NULL;
const char* candles_path = "candles_AAA_5M";

int post_aaa_5m_candle(ohlc_t entry)
{
    memmove(candles->data + candles->size, &entry, sizeof(ohlc_t));
    candles->size++;
    return 0;
}

int main()
{
    provider = SocketProviderAPI;
    tsdb = ShMapTimeseries;
    ts_iter = ShMapTsIterator;

    size_t shmap_size = sizeof(ohlc_t) * 4096 + sizeof(candle_shmap_t);
    int fd = open(candles_path, O_CREAT | O_RDWR, 0644);
    // circumvent bus error (because screw ftruncate)
    lseek(fd, shmap_size - 1, SEEK_SET);
    write(fd, "!", 1);
    lseek(fd, shmap_size - 1, SEEK_SET);
    write(fd, "\0", 1);
    lseek(fd, 4, SEEK_SET);

    candles = mmap(NULL, shmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
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

    for (int i = 0; i < candles->size; i++)
    {
        ohlc_t candle = candles->data[i];
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
