#ifndef alg_h
#define alg_h

#include "market_data.h"


typedef enum {MSG_TRADE, MSG_DEPTH} message_type_e;
typedef struct
{
    message_type_e type;
    union {
        trade_t trade;
        market_depth_t depth;
    } event;
} parsed_message_t;


#endif // alg_h