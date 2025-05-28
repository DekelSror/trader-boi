#ifndef utils_h
#define utils_h

#include <string.h>
#include <stdio.h>
#include <sys/time.h>

static inline int strneq(const char* s1, const char* s2, size_t n)
{
    return strncmp(s1, s2, n) == 0;
}

static inline long utc_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
}

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__);


#endif // utils_h