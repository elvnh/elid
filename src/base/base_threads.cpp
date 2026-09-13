#include "base_threads.h"

// NOTE: this is done to avoid a circular dependency
// TODO: solve this more properly
struct Arena;
struct Temp;
#include "base_thread_context.h"

void *thread_initialization(void *data)
{
    ThreadInitializationArgs *args = (ThreadInitializationArgs *)data;

    thread_ctx_init();

    args->entry_point(args->user_data);

    return 0;
}
