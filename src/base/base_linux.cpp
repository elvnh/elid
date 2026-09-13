#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

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
    long size = sysconf(_SC_PAGESIZE);
    ASSERT_ALWAYS(size != -1);

    usize result = (usize)size;

    return result;
}

Thread thread_create(ThreadEntryPoint entry_point, void *user_data)
{
    pthread_t thread = 0;

    // Before passing control to the entry point, run a function that initializes
    // thread local variables.
    ThreadInitializationArgs *init_args =
        (ThreadInitializationArgs *)allocate_memory(sizeof(ThreadInitializationArgs));
    init_args->entry_point = entry_point;
    init_args->user_data = user_data;

    int create_result = pthread_create(&thread, 0, thread_initialization, init_args);
    ASSERT_ALWAYS(create_result == 0);

    // We hold on to initialization arguments since we need to free them later.
    Thread result = {(u64)thread, init_args};

    return result;
}

b32 thread_join(Thread thread)
{
    pthread_t handle = (pthread_t)thread.data;
    int join_result = pthread_join(handle, 0);
    ASSERT(errno == 0);

    b32 result = join_result == 0;

    free_memory(thread.init_args, sizeof(*thread.init_args));

    return result;
}

Semaphore sema_create(u32 initial_count)
{
    // NOTE: POSIX semaphores can't be copied, which is why we have to allocate them on the
    // heap. The reason for not using arenas is that it would create a circular dependency, as
    // arenas depend on this file.
    sem_t *sem = (sem_t *)allocate_memory(sizeof(sem_t));
    ASSERT_ALWAYS(sem);

    int init_result = sem_init(sem, 0, initial_count);
    ASSERT_ALWAYS(init_result == 0);

    Semaphore result = {(u64)sem};

    return result;
}

void sema_destroy(Semaphore sema)
{
    sem_t *handle = (sem_t *)sema.data;
    sem_destroy(handle);
    free_memory(handle, sizeof(*handle));
}

void sema_inc(Semaphore sema)
{
    sem_t *handle = (sem_t *)sema.data;

    int post_result = sem_post(handle);

    ASSERT_ALWAYS(post_result == 0);
}

void sema_dec(Semaphore sema)
{
    sem_t *handle = (sem_t *)sema.data;

    int wait_result = sem_wait(handle);
    ASSERT_ALWAYS(wait_result == 0);
}

Mutex mutex_create()
{
    pthread_mutex_t *handle = (pthread_mutex_t *)allocate_memory(sizeof(pthread_mutex_t));
    ASSERT_ALWAYS(handle);

    pthread_mutexattr_t attr = {};
    int attr_init_result = pthread_mutexattr_init(&attr);
    ASSERT_ALWAYS(attr_init_result == 0);

    int attr_set_result = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ASSERT_ALWAYS(attr_set_result == 0);

    pthread_mutex_init(handle, &attr);

    // TODO: is it fine to destroy the attributes here?
    int attr_destroy_result = pthread_mutexattr_destroy(&attr);
    ASSERT_ALWAYS(attr_destroy_result == 0);

    Mutex result = {(u64)handle};

    return result;
}

void mutex_destroy(Mutex mutex)
{
    pthread_mutex_t *handle = (pthread_mutex_t *)mutex.data;
    ASSERT(handle);

    int destroy_result = pthread_mutex_destroy(handle);
    ASSERT_ALWAYS(destroy_result == 0);

    free_memory(handle, sizeof(*handle));
}

void mutex_lock(Mutex mutex)
{
    pthread_mutex_t *handle = (pthread_mutex_t *)mutex.data;
    ASSERT(handle);

    int lock_result = pthread_mutex_lock(handle);
    ASSERT_ALWAYS(lock_result == 0);
}

void mutex_unlock(Mutex mutex)
{
    pthread_mutex_t *handle = (pthread_mutex_t *)mutex.data;
    ASSERT(handle);

    int unlock_result = pthread_mutex_unlock(handle);
    ASSERT_ALWAYS(unlock_result == 0);
}

ConditionVariable cond_var_create()
{
    pthread_cond_t *data = (pthread_cond_t *)allocate_memory(sizeof(pthread_cond_t));
    ASSERT_ALWAYS(data);

    int init_cond_result = pthread_cond_init(data, 0);
    ASSERT_ALWAYS(init_cond_result == 0);

    ConditionVariable result = {(u64)data};

    return result;
}

void cond_var_destroy(ConditionVariable cond_var)
{
    pthread_cond_t *data = (pthread_cond_t *)cond_var.data;
    ASSERT(data);

    int destroy_result = pthread_cond_destroy(data);
    ASSERT(destroy_result == 0);

    free_memory(data, sizeof(*data));
}

void cond_var_signal(ConditionVariable cond_var)
{
    pthread_cond_t *data = (pthread_cond_t *)cond_var.data;
    ASSERT(data);

    int signal_result = pthread_cond_signal(data);
    ASSERT_ALWAYS(signal_result == 0);
}

void cond_var_broadcast(ConditionVariable cond_var)
{
    pthread_cond_t *data = (pthread_cond_t *)cond_var.data;
    ASSERT(data);

    int signal_result = pthread_cond_broadcast(data);
    ASSERT_ALWAYS(signal_result == 0);
}

void cond_var_wait(ConditionVariable cond_var, Mutex mutex)
{
    pthread_cond_t *cond_data = (pthread_cond_t *)cond_var.data;
    ASSERT(cond_data);
    pthread_mutex_t *mutex_data = (pthread_mutex_t *)mutex.data;
    ASSERT(mutex_data);

    int wait_result = pthread_cond_wait(cond_data, mutex_data);
    ASSERT_ALWAYS(wait_result == 0);
}

void sleep_ms(u32 ms)
{
    struct timespec ts = {};
    ts.tv_nsec = ms * 1000 * 1000;

    nanosleep(&ts, 0);
}

u64 get_monotonic_clock_ns(void)
{
    struct timespec ts = {};
    int gettime_result = clock_gettime(CLOCK_MONOTONIC, &ts);
    ASSERT_ALWAYS(gettime_result == 0);

    u64 result = (u64)ts.tv_nsec;

    return result;
}

u64 get_monotonic_clock_ms(void)
{
    u64 ns = get_monotonic_clock_ns();
    u64 result = ns_to_ms(ns);

    return result;
}
