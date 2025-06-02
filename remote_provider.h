#ifndef remote_provider_h
#define remote_provider_h

#include "market_data.h"

typedef struct
{
    int(*connect)();
    int(*get_msg)(parsed_message_t* buf);
    int(*disconnect)();
} remote_provider_api_t;

extern const remote_provider_api_t SocketProviderAPI;

#endif // remote_provider_h