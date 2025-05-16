
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "config_utils.h"

char* get_config_value(const char* key)
{
    int config_fd = open("./config.conf", O_RDONLY);
    char config_buf[4096];
    read(config_fd, config_buf, 4096);
    char* p = config_buf;
    while (*p)
    {
        char* key_start = p;
        while (*p && *p != '=') p++;

        if (strncmp(key_start, key, p - key_start) == 0)
        {
            char* val_end = p;
            while (*val_end && *val_end != '\n') val_end++;
            val_end--;
            char* val_buf = malloc(val_end - p + 1);
            strncpy(val_buf, p + 1, val_end - p);
            val_buf[val_end - p] = 0;
            close(config_fd);
            return val_buf;
        }
        else
        {
            while (*p && *p != '\n') p++;
        }
    }

    return NULL;
    close(config_fd);
}
