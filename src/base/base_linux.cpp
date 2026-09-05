#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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

Thread_Maybe thread_create(ThreadEntryPoint entry_point, void *user_data)
{
    Thread_Maybe result = {};

    pthread_t thread = 0;
    int create_result = pthread_create(&thread, 0, entry_point, user_data);

    if (create_result == 0) {
        result.ok = true;
        result.value.data = thread;
    }

    return result;
}

b32 thread_join(Thread thread)
{
    pthread_t handle = (pthread_t)thread.data;
    int join_result = pthread_join(handle, 0);

    b32 result = join_result == 0;

    return result;
}

Semaphore_Maybe sema_create(u32 initial_count)
{
    Semaphore_Maybe result = {};

    // NOTE: POSIX semaphores can't be copied, which is why we have to allocate them on the
    // heap. The reason for not using arenas is that it would create a circular dependency, as
    // arenas depend on this file.
    sem_t *sem = (sem_t *)allocate_memory(sizeof(sem_t));

    if (sem) {
        int init_result = sem_init(sem, 0, initial_count);

        if (init_result == 0) {
            result = SOME({(u64)sem});
        } else {
            free_memory(sem, sizeof(sem_t));
        }
    }

    return result;
}

void sema_free(Semaphore sema)
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

struct lnx_Mutex {
    pthread_mutex_t handle;
    pthread_mutexattr_t attributes;
};

Mutex mutex_create()
{
    lnx_Mutex *data = (lnx_Mutex *)allocate_memory(sizeof(lnx_Mutex));
    ASSERT_ALWAYS(data);

    int attr_init_result = pthread_mutexattr_init(&data->attributes);
    ASSERT_ALWAYS(attr_init_result == 0);

    int setattr_result = pthread_mutexattr_settype(&data->attributes, PTHREAD_MUTEX_RECURSIVE);
    ASSERT_ALWAYS(setattr_result == 0);

    pthread_mutex_init(&data->handle, &data->attributes);

    Mutex result = {(u64)data};

    return result;
}

void mutex_free(Mutex mutex)
{
    lnx_Mutex *data = (lnx_Mutex *)mutex.data;
    ASSERT(data);

    int destroy_attr_result = pthread_mutexattr_destroy(&data->attributes);
    int destroy_mutex_result = pthread_mutex_destroy(&data->handle);

    ASSERT_ALWAYS(destroy_attr_result == 0);
    ASSERT_ALWAYS(destroy_mutex_result == 0);

    free_memory(data, sizeof(*data));
}

void mutex_lock(Mutex mutex)
{
    lnx_Mutex *data = (lnx_Mutex *)mutex.data;
    ASSERT(data);

    int lock_result = pthread_mutex_lock(&data->handle);
    ASSERT_ALWAYS(lock_result == 0);
}

void mutex_unlock(Mutex mutex)
{
    lnx_Mutex *data = (lnx_Mutex *)mutex.data;
    ASSERT(data);

    int unlock_result = pthread_mutex_unlock(&data->handle);
    ASSERT_ALWAYS(unlock_result == 0);
}
