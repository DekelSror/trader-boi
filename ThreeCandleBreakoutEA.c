

/*
temp typedefs
*/
typedef void* candlestick_t;
typedef void* moving_avg_t;
extern(*get_agg)(const char* agg, const char* sym, ...);


/* "compiled" C code (from ThreeCandleBreakoutEA.json) */
static const char* symbol;
static candlestick_t candles;
static moving_avg_t ema;

void init()
{
    candles = get_agg("candlestick", symbol, "5M");
    ema = get_agg("moving_avg", symbol, "10M", "exponential");
}

void on_trade()
{
}
