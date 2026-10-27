#pragma once

/* Threads */
typedef void *(*ThreadEntryPoint)(void *user_data);

struct ThreadInitializationArgs {
    ThreadEntryPoint entry_point;
    void *user_data;
};
struct Thread {
    u64 data;
    ThreadInitializationArgs *init_args;
};

Thread thread_create(ThreadEntryPoint entry_point, void *user_data);
b32    thread_join(Thread thread);

void  *thread_initialization(void *data);

/* Semaphores */
struct Semaphore {
    u64 data;
};

Semaphore sema_create(u32 initial_count);
void      sema_destroy(Semaphore sema);
void      sema_inc(Semaphore sema);
void      sema_dec(Semaphore sema);
s32       sema_get(Semaphore sema);

/* Recursive mutexes */
struct Mutex {
    u64 data;
};

Mutex mutex_create(void);
void  mutex_destroy(Mutex mutex);
void  mutex_lock(Mutex mutex);
void  mutex_unlock(Mutex mutex);

/* Scoped lock */
struct ScopedLock {
    Mutex mutex;

    ~ScopedLock(void)
    {
        ASSERT(mutex.data);

        mutex_unlock(mutex);

        #if DEBUG_BUILD
        mutex = {};
        #endif
    }
};

ScopedLock scoped_lock(Mutex mutex)
{
    mutex_lock(mutex);
    return {mutex};
}

/* Condition variables */
struct ConditionVariable {
    u64 data;
};

ConditionVariable cond_var_create(void);
void              cond_var_destroy(ConditionVariable cond_var);
void              cond_var_signal(ConditionVariable cond_var);
void              cond_var_broadcast(ConditionVariable cond_var);
void              cond_var_wait(ConditionVariable cond_var, Mutex mutex);

/* Atomics */
#if COMPILER_GCC
#    define atomic_load_s32(ptr)          __atomic_load_n((ptr), __ATOMIC_SEQ_CST)
#    define atomic_store_s32(ptr, val)    __atomic_store_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_add_s32(ptr, val)      __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_sub_s32(ptr, val)      __atomic_sub_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_inc_s32(ptr)           __atomic_add_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_dec_s32(ptr)           __atomic_sub_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_exchange_s32(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_compare_exchange_s32(ptr, expect, desired)           \
    __atomic_compare_exchange_n((ptr), (expect), (desired), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#
#    define atomic_load_u32(ptr)          __atomic_load_n((ptr), __ATOMIC_SEQ_CST)
#    define atomic_store_u32(ptr, val)    __atomic_store_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_add_u32(ptr, val)      __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_sub_u32(ptr, val)      __atomic_sub_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_inc_u32(ptr)           __atomic_add_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_dec_u32(ptr)           __atomic_sub_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_exchange_u32(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_compare_exchange_u32(ptr, expect, desired)         \
    __atomic_compare_exchange_n((ptr), (expect), (desired), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#
#    define atomic_load_ptr(ptr)          __atomic_load_n((ptr), __ATOMIC_SEQ_CST)
#    define atomic_store_ptr(ptr, val)    __atomic_store_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_add_ptr(ptr, val)      __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_sub_ptr(ptr, val)      __atomic_sub_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_inc_ptr(ptr)           __atomic_add_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_dec_ptr(ptr)           __atomic_sub_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_exchange_ptr(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_compare_exchange_ptr(ptr, expect, desired)         \
    __atomic_compare_exchange_n((ptr), (expect), (desired), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#
#    define atomic_load_usize(ptr)          __atomic_load_n((ptr), __ATOMIC_SEQ_CST)
#    define atomic_store_usize(ptr, val)    __atomic_store_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_add_usize(ptr, val)      __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_sub_usize(ptr, val)      __atomic_sub_fetch((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_inc_usize(ptr)           __atomic_add_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_dec_usize(ptr)           __atomic_sub_fetch((ptr), 1, __ATOMIC_SEQ_CST)
#    define atomic_exchange_usize(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_SEQ_CST)
#    define atomic_compare_exchange_usize(ptr, expect, desired)         \
    __atomic_compare_exchange_n((ptr), (expect), (desired), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#else
#    error Atomics not defined for this compiler.
#endif
