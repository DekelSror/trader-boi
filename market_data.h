#ifndef market_data_h
#define market_data_h

#include <stdint.h>
// Common market data structures following FIX/FAST protocol conventions

// Trade data (Time & Sales)
typedef struct {
    long timestamp;      // Microseconds since epoch
    double price;           // Trade price
    int32_t volume;         // Trade volume
    char side;              // 'B' for buy, 'S' for sell
    char condition[4];      // Trade condition (e.g., "REG", "CXL")
    char symbol[16];        // Ticker symbol (e.g., "AAPL", "MSFT")
} trade_t;

// Level 2 Market Data (Order Book)
typedef struct {
    long timestamp;      // Microseconds since epoch
    double price;           // Price level
    int32_t size;           // Total size at this price level
    int32_t orders;         // Number of orders at this price level
    char side;              // 'B' for bid, 'A' for ask
} order_book_level_t;

// Market Depth Snapshot
typedef struct {
    long timestamp;      // Microseconds since epoch
    order_book_level_t bids[10];  // Top 10 bid levels
    order_book_level_t asks[10];  // Top 10 ask levels
    double spread;          // Current spread
    char symbol[16];        // Ticker symbol
} market_depth_t;

typedef enum {MSG_TRADE, MSG_DEPTH} message_type_e;
typedef struct
{
    message_type_e type;
    union {
        trade_t trade;
        market_depth_t depth;
    } event;
} parsed_message_t;


typedef struct
{
    double open;
    double close;
    double high;
    double low;
    long timestamp;
    char symbol[8];
} ohlc_t;

#endif // market_data_h 