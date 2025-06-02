#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "shmap.h"

// trucnates the file if exists
void* shmap_create(const char* path, size_t initial_size)
{
    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    // circumvent bus error (because screw ftruncate)
    lseek(fd, initial_size - 1, SEEK_SET);
    write(fd, "!", 1);
    lseek(fd, initial_size - 1, SEEK_SET);
    write(fd, "\0", 1);
    lseek(fd, 0, SEEK_SET);

    void* res = mmap(NULL, initial_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    return res;
}


void* shmap_reader(const char* path)
{
    int fd = open(path, O_RDONLY, 0400);

    struct stat st_path;
    stat(path, &st_path);

    void* res = mmap(NULL, st_path.st_size, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);

    return res;
}
