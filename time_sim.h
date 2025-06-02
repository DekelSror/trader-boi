#ifndef time_simulator_h
#define time_simulator_h


typedef struct {
    int total_ticks;
    long tick_interval_us;
    long base_time;
} time_simulator_t;

time_simulator_t time_sim_init(int total_ticks, long tick_interval_us);
long time_sim_get_tick(time_simulator_t sim, int tick_index);

#endif // time_simulator_h