#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "market_data.h" // trade_t, market_depth_t, parsed_message_t

#define MQ_NAME "/parsed_market_data"
#define MQ_MAX_MESSAGES 10
#define MQ_MAX_MSG_SIZE sizeof(parsed_message_t)

typedef struct
{
    int delta;
    int duration;
} value_delta_t;


value_delta_t aaa_deltas[] = {
    (value_delta_t){5, 64},
    (value_delta_t){-5, 64},
};


value_delta_t bbb_deltas[] = {
    (value_delta_t){10, 32},
    (value_delta_t){-10, 32},
    (value_delta_t){0, 32},
    (value_delta_t){8, 32},
};

int mock_size = 128;

typedef struct
{
    int current_price;
    int current_delta;
    int current_duration;
    int num_deltas;
    value_delta_t* deltas;
    char* symbol;
} mocker_t;

void tick(mocker_t* mock)
{
    if (mock->current_delta == mock->num_deltas)
        return;

    if (mock->current_duration == 0)
    {
        mock->current_delta++;
        mock->current_duration = mock->deltas[mock->current_delta].duration;
    }

    mock->current_price += mock->deltas[mock->current_delta].delta;
    mock->current_duration--;
}

static long get_timestamp_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

parsed_message_t create_trade(mocker_t mock, long timestamp)
{
    parsed_message_t msg = {
        .type = MSG_TRADE,
        .event.trade.timestamp = timestamp,
        .event.trade.price = (double)mock.current_price,
        .event.trade.volume = 1,
        .event.trade.side = (mock_size % 2 == 0) ? 'B' : 'S',
    };

    strncpy(msg.event.trade.condition, "REG", 4);
    strncpy(msg.event.trade.symbol, mock.symbol, 4);

    return msg;
}


void trade_print(parsed_message_t msg)
{
    printf("Tick %d (%s): Price: %.2f, Vol: %d, Side: %c\n",
        128 - mock_size,
        msg.event.trade.symbol,
        msg.event.trade.price,
        msg.event.trade.volume,
        msg.event.trade.side
    );
}

int main(int argc, char const *argv[])
{
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = "/tmp/trader-boi-comm",
    };

    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    mocker_t aaa_mocker = {
        .current_price = 100,
        .current_delta = 0,
        .current_duration = aaa_deltas[0].duration,
        .deltas = aaa_deltas,
        .num_deltas = 2,
        .symbol = "AAA",
    };

    mocker_t bbb_mocker = {
        .current_price = 100,
        .current_delta = 0,
        .current_duration = bbb_deltas[0].duration,
        .deltas = bbb_deltas,
        .num_deltas = 4,
        .symbol = "BBB",
    };

    ssize_t dbg = -1;

    getc(stdin);

    while (mock_size--)
    {
        tick(&aaa_mocker);
        tick(&bbb_mocker);

        long ts = get_timestamp_us();

        parsed_message_t aaa_msg = create_trade(aaa_mocker, ts);
        trade_print(aaa_msg);

        dbg = sendto(sock, &aaa_msg, sizeof(aaa_msg), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        

        parsed_message_t bbb_msg = create_trade(bbb_mocker, ts);
        trade_print(bbb_msg);
        
        dbg = sendto(sock, &bbb_msg, sizeof(bbb_msg), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        
        const struct timespec nsleeptime = {.tv_nsec=100000000};
        nanosleep(&nsleeptime, NULL);
    }

    printf("Finished mocking data.\n");

    // close queue
    // if (mq_close(mqd) == -1) {
    //     perror("mq_close failed");
    // } else {
    //     printf("Message queue %s closed.\n", MQ_NAME);
    // }

    // if (mq_unlink(MQ_NAME) == -1) {
    //     perror("mq_unlink failed");
    // } else {
    //     printf("Message queue %s unlinked.\\n", MQ_NAME);
    // }

    close(sock);
    
    return 0;
}
