#pragma once

/* Threads */
struct Thread {
    u64 data;
};

DEFINE_MAYBE_TYPE(Thread);

typedef void *(*ThreadEntryPoint)(void *user_data);

Thread_Maybe thread_create(ThreadEntryPoint entry_point, void *user_data);
b32          thread_join(Thread thread);

/* Semaphores */
struct Semaphore {
    u64 data;
};

DEFINE_MAYBE_TYPE(Semaphore);

Semaphore_Maybe sema_create(u32 initial_count);
void            sema_free(Semaphore sema);
void            sema_inc(Semaphore sema);
void            sema_dec(Semaphore sema);
