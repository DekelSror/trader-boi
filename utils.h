#ifndef utils_h
#define utils_h

#include <string.h>
#include <stdio.h>

static inline int strneq(const char* s1, const char* s2, size_t n)
{
    return strncmp(s1, s2, n) == 0;
}

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__);


#endif // utils_h