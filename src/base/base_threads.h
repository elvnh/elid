#pragma once

/* Threads */
struct Thread {
    u64 data;
};

// TODO: don't use maybe types, if any of these creation functions fails we can't recover
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

/* Recursive mutexes */
struct Mutex {
    u64 data;
};

Mutex mutex_create();
void  mutex_free(Mutex mutex);
void  mutex_lock(Mutex mutex);
void  mutex_unlock(Mutex mutex);

/* Condition variables */
struct ConditionVariable {
    u64 data;
};

ConditionVariable cond_var_create();
void              cond_var_free(ConditionVariable cond_var);
void              cond_var_signal(ConditionVariable cond_var);
void              cond_var_broadcast(ConditionVariable cond_var);
void              cond_var_wait(ConditionVariable cond_var, Mutex mutex);
