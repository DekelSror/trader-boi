#ifndef shared_mmap_h
#define shared_mmap_h

#include <stddef.h>

void* shmap_create(const char* path, size_t initial_size);
void* shmap_reader(const char* path);

#endif // shared_mmap_h