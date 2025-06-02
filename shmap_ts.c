#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "shmap.h"
#include "timeseries.h"

typedef struct
{
    ts_entry_t* mem;
    int size;
    char name[128];
} shmap_ts_t;

shmap_ts_t db[16];
int num_series = 0;

static int find_series(const char* name)
{
    int i = 0;
    for (; i < num_series; i++)
    {
        if (strneq(name, db[i].name, 128))
        {
            break;
        }
    }

    if (i == num_series + 1)
        return -1;
    

    return i;
}

static int create(const char* name)
{
    if (num_series == 16)
        return 1;

    char path[12];
    sprintf(path, "shmap_ts_%02d", num_series);
    memmove(db[num_series].name, name, 128);
    
    
    db[num_series].mem = shmap_create(path, sizeof(ts_entry_t) * 4096);
    num_series++;

    return 0;
}

static int add(const char* name, long timestamp, double val)
{
    int i = find_series(name);

    if (i == -1)
        return 1;
    
    if (db[i].size == 4096)
        return 2;
    
    db[i].mem[db[i].size].t = timestamp;
    db[i].mem[db[i].size].v = val;
    db[i].size++;

    return 0;
}

typedef struct
{
    int series;
    long start;
    long end;
    long current;
} shmap_ts_iterator_t;

static void* frame_by_index(const char* name, long start, long end)
{
    int i = find_series(name);

    if (i == -1)
        return NULL;

    shmap_ts_iterator_t* iter = malloc(sizeof(*iter));

    if (start < 0)
        start = 0;
    if (end > db[i].size || end < 0)
        end = db[i].size;

    iter->current = start;
    iter->start = start;
    iter->end = end;
    iter->series = i;

    return (void*)iter;
}

static void* frame_by_times(const char* name, long start, long end)
{
    (void)name;
    (void)start;
    (void)end;
    return NULL;
}

const timeseries_api_t ShMapTimeseries = {
    .create=create,
    .add=add,
    .frame_by_index=frame_by_index,
    .frame_by_times=frame_by_times,
};

static int is_spent(void* iter_)
{
    shmap_ts_iterator_t* iter = iter_;

    return iter->current == iter->end;
}

static ts_entry_t next(void* iter_)
{
    shmap_ts_iterator_t* iter = iter_;
    ts_entry_t rv = db[iter->series].mem[iter->current];

    iter->current++;

    return rv;
}


const ts_iterator_api_t ShMapTsIterator = {
    .is_spent=is_spent,
    .next=next,
};