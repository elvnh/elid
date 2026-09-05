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

/* Atomics */
#if COMPILER_GCC
#    define atomic_load_u32(ptr)          __atomic_load_n((ptr), __ATOMIC_SEQ_CST)
#    define atomic_store_u32(ptr, val)    __atomic_store_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_add_u32(ptr, val)      __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_sub_u32(ptr, val)      __atomic_sub_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_inc_u32(ptr)           __atomic_add_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_dec_u32(ptr)           __atomic_sub_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_exchange_u32(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_SEQ_CST)
#else
#    error Atomics not defined for this compiler.
#endif
