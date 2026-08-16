#include <sys/mman.h>
#include <unistd.h>

void *allocate_memory(usize size)
{
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (NEVER(result == MAP_FAILED)) {
        result = 0;
    }

    return result;
}

void free_memory(void *ptr, usize size)
{
    int result = munmap(ptr, size);
    ASSERT(result == 0);
}

usize get_system_page_size(void)
{
    // TODO: cache system info
    // TODO: if this call fails, abort program
    long size = sysconf(_SC_PAGESIZE);
    ASSERT_ALWAYS(size != -1);

    usize result = (usize)size;

    return result;
}
