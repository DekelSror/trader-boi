#include <stdint.h>
#include <string.h>
#include <time.h>

#include "nx_parsers.h"

static int64_t nxcore_to_unix_timestamp(const NxDate* date, const NxTime* time)
{
    struct tm tm = {0};
    tm.tm_year = date->Year - 1900;
    tm.tm_mon = date->Month - 1;
    tm.tm_mday = date->Day;
    tm.tm_hour = time->Hour;
    tm.tm_min = time->Minute;
    tm.tm_sec = time->Second;
    
    time_t unix_time = mktime(&tm);
    return (int64_t)unix_time * 1000000LL + time->Millisecond * 1000LL;
}

static double nxcore_to_price(int price, unsigned char price_type)
{
    return price / 10000.0;
}

void nxcore_to_trade(const struct NxCoreMessage* msg, const struct NxCoreTrade* nx_trade, trade_t* trade)
{
    trade->timestamp = nxcore_to_unix_timestamp(
        &msg->coreHeader.nxSessionDate, 
        &msg->coreHeader.nxExgTimestamp);
    trade->price = nxcore_to_price(nx_trade->Price, nx_trade->PriceType);
    trade->volume = nx_trade->Size;
    trade->side = nx_trade->BATECode;  // 'B' for bid, 'A' for ask, 'T' for trade
    strncpy(trade->condition, "REG", sizeof(trade->condition));
    
    // Copy the symbol (if available)
    if (msg->coreHeader.pnxStringSymbol) {
        strncpy(trade->symbol, msg->coreHeader.pnxStringSymbol->String, sizeof(trade->symbol) - 1);
        trade->symbol[sizeof(trade->symbol) - 1] = '\0'; // Ensure null termination
    } else {
        strncpy(trade->symbol, "UNKNOWN", sizeof(trade->symbol));
    }
}

// Convert NxCore quote to our order_book_level_t format
void nxcore_to_order_book_level(
    const struct NxCoreMessage* msg,
    const struct NxCoreQuote* nx_quote, 
    order_book_level_t* level,
    char side
)
{
    level->timestamp = nxcore_to_unix_timestamp(
        &msg->coreHeader.nxSessionDate,
        &msg->coreHeader.nxExgTimestamp);
    level->side = side;
    
    if (side == 'B') {
        level->price = nxcore_to_price(nx_quote->BidPrice, nx_quote->PriceType);
        level->size = nx_quote->BidSize;
        level->orders = 1;  // NxCore doesn't provide order count, default to 1
    } else {
        level->price = nxcore_to_price(nx_quote->AskPrice, nx_quote->PriceType);
        level->size = nx_quote->AskSize;
        level->orders = 1;
    }
}

// Convert NxCore market depth to our market_depth_t format
void nxcore_to_market_depth(
    const struct NxCoreMessage* msg,
    const struct NxCoreExgQuote* nx_depth,
    market_depth_t* depth
)
{
    // Get timestamp from the message header
    depth->timestamp = nxcore_to_unix_timestamp(&msg->coreHeader.nxSessionDate,
                                              &msg->coreHeader.nxExgTimestamp);
    
    // Convert top level quote
    nxcore_to_order_book_level(msg, &nx_depth->coreQuote, &depth->bids[0], 'B');
    nxcore_to_order_book_level(msg, &nx_depth->coreQuote, &depth->asks[0], 'A');
    
    // Calculate spread
    depth->spread = nxcore_to_price(nx_depth->BestAskPrice, 
                                  nx_depth->coreQuote.PriceType) -
                   nxcore_to_price(nx_depth->BestBidPrice, 
                                  nx_depth->coreQuote.PriceType);
    
    // Copy the symbol (if available)
    if (msg->coreHeader.pnxStringSymbol) {
        strncpy(depth->symbol, msg->coreHeader.pnxStringSymbol->String, sizeof(depth->symbol) - 1);
        depth->symbol[sizeof(depth->symbol) - 1] = '\0'; // Ensure null termination
    } else {
        strncpy(depth->symbol, "UNKNOWN", sizeof(depth->symbol));
    }
}
