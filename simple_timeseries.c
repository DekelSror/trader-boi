#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"
#include "simple_timeseries.h"


static int next_series = 0;
static double* values[MAX_SERIES];
static long* times[MAX_SERIES];
static char* series_names[MAX_SERIES];
static long add_indices[MAX_SERIES];


static int create(const char* name)
{
    if (next_series == MAX_SERIES)
    {
        return -1;
    }

    size_t namelen = strlen(name);
    if (namelen < 1024)
    {
        series_names[next_series] = malloc(namelen);
        memmove(series_names[next_series], name, namelen);
    }

    values[next_series] = malloc(1024 * sizeof(long));
    times[next_series] = malloc(1024 * sizeof(long));

    int rv = next_series;
    next_series++;
    return rv;
}

static int series_index_by_name(const char* name)
{
    int i = 0;
    for (; i < MAX_SERIES; i++)
    {
        if (strneq(series_names[i], name, 1024))
        {
            break;
        }
    }

    if (i == MAX_SERIES) return -1;

    return i;
    
}

static int add(const char* series_name, long timestamp, double val)
{
    int i = series_index_by_name(series_name);

    if (i == -1) return 1;
    if (add_indices[i] == 1024) return 2;

    values[i][add_indices[i]] = val;
    if (timestamp > 0)
    {
        times[i][add_indices[i]] = timestamp;
    }
    else
    {
        times[i][add_indices[i]] = utc_now();
    }
    add_indices[i]++;

    return 0;
}


typedef struct
{
    int series;
    long start;
    long end;
    long current;
} sts_iterator_t;


static void* frame_by_index(const char* series_name, long start, long end)
{
    int i = series_index_by_name(series_name);
    if (i == -1) return NULL;

    if (start < 0)
        start = 0;
    if (end > 1024 || end <= 0)
        end = add_indices[i];

    sts_iterator_t* res = malloc(sizeof(*res));

    res->series = i;
    res->start = start;
    res->end = end;
    res->current = start;

    return (void*)res;

}



static void* frame_by_times(const char* series_name, long start, long end)
{
    (void)series_name;
    (void)start;
    (void)end;
    return NULL;
}


static int is_spent(void* iter_)
{
    sts_iterator_t* iter = iter_;
    return iter->current == iter->end;
}

static ts_entry_t next(void* iter_)
{
    sts_iterator_t* iter = iter_;
    double rv = values[iter->series][iter->current];
    long rt = times[iter->series][iter->current];
    iter->current++;

    return (ts_entry_t){.v = rv, .t = rt};  
}

const timeseries_api_t SimpleTimeseries = {
    .create=create,
    .add=add,
    .frame_by_index=frame_by_index,
    .frame_by_times=frame_by_times,
};

const ts_iterator_api_t SimpleTimeseriesIterator = {
    .is_spent=is_spent,
    .next=next,
};