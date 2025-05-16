#ifndef nx_parsers_h
#define nx_parsers_h


#include "market_data.h"
#include "NxCoreAPI.h"


void nxcore_to_market_depth(
    const struct NxCoreMessage* msg,
    const struct NxCoreExgQuote* nx_depth,
    market_depth_t* depth
);

void nxcore_to_order_book_level(
    const struct NxCoreMessage* msg,
    const struct NxCoreQuote* nx_quote, 
    order_book_level_t* level,
    char side
);

void nxcore_to_trade(
    const struct NxCoreMessage* msg,
    const struct NxCoreTrade* nx_trade,
    trade_t* trade
);



#endif // nx_parsers_h