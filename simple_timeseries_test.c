#include <stdio.h>
#include <unistd.h>

#include "simple_timeseries.h"


int main()
{
    timeseries_api_t ts_class = SimpleTimeseries;
    ts_iterator_api_t ts_iter_class = SimpleTimeseriesIterator; 

    ts_class.create("test");

    ts_class.add("test", -1, 1.0);
    usleep(5000);
    ts_class.add("test", -1, 20.0);
    usleep(5000);
    ts_class.add("test", -1, 300.0);
    ts_class.add("test", -1, 4.05);
    usleep(5000);
    ts_class.add("test", -1, 6.007);

    void* iter = ts_class.frame_by_index("test", 2, 4);
    int i = 2;
    while (!ts_iter_class.is_spent(iter))
    {
        ts_entry_t entry = ts_iter_class.next(iter);
        printf("elem %d %lf\n", i, entry.v);
        i++;
    }
    return 0;
}
