#include <sys/time.h>
#include <stddef.h>

#include "time_sim.h"


time_simulator_t time_sim_init(int total_ticks, long tick_interval_us)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    time_simulator_t sim = {
        .total_ticks = total_ticks,
        .tick_interval_us = tick_interval_us,
        .base_time = tv.tv_sec * 1000000L + tv.tv_usec,
    };
    
    return sim;
}

long time_sim_get_tick(time_simulator_t sim, int tick_index)
{
    return sim.base_time + (tick_index * sim.tick_interval_us);
}
