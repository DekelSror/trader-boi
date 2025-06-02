#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "time_sim.h"
#include "market_data.h" // trade_t, market_depth_t, parsed_message_t
#include "mock_server.h"

#define TIME_ACCELERATION_FACTOR 100 
#define TOTAL_TICKS 75 // Total number of ticks to simulate


typedef struct {
    double trend;
    int trend_age;
    char* symbol;
    double current_price;
} price_simulator_t;

price_simulator_t create_simulator(char* symbol, double base_price) {
    price_simulator_t sim = {
        .trend = 10.0,
        .symbol = symbol,
        .current_price = base_price,
        .trend_age = 0,
    };
    
    return sim;
}

double generate_next_price(price_simulator_t sim) {
    sim.trend_age++;
    sim.current_price = sim.current_price + sim.trend;
    
    if (sim.trend_age == 20)
    {
        sim.trend_age = 0;
        sim.trend = sim.trend * -1;
    }
    if (sim.current_price <= 0) {
        sim.current_price = 0;
    }
    
    return sim.current_price;
}

parsed_message_t create_trade(price_simulator_t sim, long timestamp) {
    parsed_message_t msg = {
        .type = MSG_TRADE,
        .event.trade.timestamp = timestamp,
        .event.trade.price = generate_next_price(sim),
        .event.trade.volume = (rand() % 10) + 1,
        .event.trade.side = (sim.trend > 0.0) ? 'B' : 'S',
    };

    strncpy(msg.event.trade.condition, "REG", 4);
    strncpy(msg.event.trade.symbol, sim.symbol, 4);

    return msg;
}

void trade_print(parsed_message_t msg) {
    time_t seconds = msg.event.trade.timestamp / 1000000;
    struct tm *timeinfo = localtime(&seconds);
    char time_buffer[80];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    printf("%s (%s): %s.%06ld, Price: %.2f, Vol: %d\n",
        msg.event.trade.symbol,
        msg.event.trade.side == 'B' ? "BUY" : "SELL",
        time_buffer,
        msg.event.trade.timestamp % 1000000,
        msg.event.trade.price,
        msg.event.trade.volume
    );
}

int main(int argc, char const *argv[])
{
    int server_sock = mock_server_init();
    int client_sock = mock_server_accept_client(server_sock);

    price_simulator_t aaa_sim = create_simulator("AAA", 100.0);
    time_simulator_t time_sim = time_sim_init(TOTAL_TICKS, 60000000);
    ssize_t dbg = -1;
    
    for (int tick_idx = 0; tick_idx < TOTAL_TICKS; tick_idx++) {
        long ts = time_sim_get_tick(time_sim, tick_idx);
        parsed_message_t aaa_msg = create_trade(aaa_sim, ts);

        trade_print(aaa_msg);

        dbg = mock_server_send_message(client_sock, &aaa_msg);
        if (dbg < 0) {
            break;
        }
        
        usleep(10000);
    }

    printf("Finished mocking data.\n");
    mock_server_cleanup(server_sock, client_sock);
    
    return 0;
}
