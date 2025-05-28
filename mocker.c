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
#include <math.h>

#include "market_data.h" // trade_t, market_depth_t, parsed_message_t

#define SOCKET_PATH "/tmp/trader-boi-comm"
#define TIME_ACCELERATION_FACTOR 100 
#define TICKS_PER_TIMEFRAME 15
#define TIMEFRAME_US 300000000  // 5 minutes in microseconds

typedef struct {
    double base_price;
    double volatility;
    double trend;
    double mean_reversion;
    char* symbol;
    
    double current_price;
    double prev_price;
} price_simulator_t;

price_simulator_t create_simulator(char* symbol, double base_price) {
    price_simulator_t sim = {
        .base_price = base_price,
        .volatility = 0.1,
        .trend = 0.0,
        .mean_reversion = 0.05,
        .symbol = symbol,
        .current_price = base_price,
        .prev_price = base_price
    };
    
    return sim;
}

double generate_next_price(price_simulator_t* sim) {
    sim->prev_price = sim->current_price;
    
    double random_factor = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
    double price_change = random_factor * sim->volatility * sim->current_price / 100.0;
    price_change += sim->trend * sim->current_price / 100.0;
    
    double distance_from_base = sim->current_price - sim->base_price;
    price_change -= distance_from_base * sim->mean_reversion / 100.0;
    
    sim->current_price += price_change;
    
    if (sim->current_price <= 0) {
        sim->current_price = sim->prev_price * 0.9;
    }
    
    if (rand() % 50 == 0) {
        double jump_factor = (((double)rand() / RAND_MAX) * 2.0 - 1.0) * 0.5;
        sim->current_price *= (1.0 + jump_factor);
    }
    
    return sim->current_price;
}

static long simulated_timestamp(int timeframe_index, int tick_within_timeframe) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long base_time = tv.tv_sec * 1000000L + tv.tv_usec;
    long tick_spacing = TIMEFRAME_US / TICKS_PER_TIMEFRAME;
    
    return base_time + 
        (timeframe_index * TIMEFRAME_US) + 
        (tick_within_timeframe * tick_spacing);
}

parsed_message_t create_trade(price_simulator_t sim, long timestamp) {
    parsed_message_t msg = {
        .type = MSG_TRADE,
        .event.trade.timestamp = timestamp,
        .event.trade.price = sim.current_price,
        .event.trade.volume = (rand() % 10) + 1,
        .event.trade.side = (sim.current_price > sim.prev_price) ? 'B' : 'S',
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
    srand(time(NULL));
    
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = SOCKET_PATH,
    };

    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    unlink(server_addr.sun_path);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Market data server started on %s. Waiting for clients...\n", SOCKET_PATH);

    int client_sock;
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Client connected. Starting data feed...\n");

    price_simulator_t aaa_sim = create_simulator("AAA", 100.0);
    price_simulator_t bbb_sim = create_simulator("BBB", 200.0);
    
    bbb_sim.volatility = 0.2;
    aaa_sim.trend = 0.05;
    bbb_sim.trend = -0.03;

    ssize_t dbg = -1;
    
    int total_timeframes = 20;
    
    for (int timeframe_idx = 0; timeframe_idx < total_timeframes; timeframe_idx++) {
        printf("\n--- Timeframe %d ---\n", timeframe_idx + 1);
        
        for (int tick_idx = 0; tick_idx < TICKS_PER_TIMEFRAME; tick_idx++) {
            generate_next_price(&aaa_sim);
            generate_next_price(&bbb_sim);

            long ts = simulated_timestamp(timeframe_idx, tick_idx);

            parsed_message_t aaa_msg = create_trade(aaa_sim, ts);
            trade_print(aaa_msg);

            dbg = send(client_sock, &aaa_msg, sizeof(aaa_msg), 0);
            if (dbg < 0) {
                perror("send failed for AAA message");
                break;
            }
            
            parsed_message_t bbb_msg = create_trade(bbb_sim, ts);
            trade_print(bbb_msg);
            
            dbg = send(client_sock, &bbb_msg, sizeof(bbb_msg), 0);
            if (dbg < 0) {
                perror("send failed for BBB message");
                break;
            }
            
            usleep(10000);
        }
    }

    printf("Finished mocking data.\n");
    close(client_sock);
    close(server_sock);
    unlink(SOCKET_PATH);
    
    return 0;
}
