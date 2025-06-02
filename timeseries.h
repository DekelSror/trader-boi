#ifndef timeseries_h
#define timeseries_h


typedef struct
{
    int(*create)(const char* name);
    int(*add)(const char* series_name, long timestamp, double val);
    void*(*frame_by_index)(const char* series_name, long start, long end);
    void*(*frame_by_times)(const char* series_name, long start, long end);
} timeseries_api_t;

typedef struct {
    long t;
    double v;
} ts_entry_t;

typedef struct
{
    int(*is_spent)(void*);
    ts_entry_t(*next)(void*);
} ts_iterator_api_t;


extern const timeseries_api_t ShMapTimeseries;
extern const ts_iterator_api_t ShMapTsIterator;

extern const timeseries_api_t SimpleTimeseries;
extern const ts_iterator_api_t SimpleTimeseriesIterator;

#endif // timeseries_h