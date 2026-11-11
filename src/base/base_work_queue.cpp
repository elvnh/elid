#include "base_work_queue.h"

#define WQ_TASK_USER_DATA_SIZE KB(4)
#define WQ_INITIAL_FREE_TASK_CAP 16
#define WQ_THREAD_WORK_QUEUE_ARENA_SIZE WQ_INITIAL_FREE_TASK_CAP * sizeof(wq__Task)
#define WQ_WAIT_FOR_TASK_TIMEOUT_IN_USEC ms_to_us(1)

// TODO: rename
struct wq__Task {
    char user_data[WQ_TASK_USER_DATA_SIZE];

    TaskFunction function;
    wq__Task *parent;
    wq__ThreadWorkQueue *from_queue;

    s32 unfinished_task_count; // Atomic, number of unfinished tasks including child tasks
    TaskGeneration generation; // Atomic, increased when a task is finished

    wq__Task *next; // Either next free task or next enqueued task

    #if DEBUG_BUILD
    b32 has_been_run;
    #endif
};

struct wq__ThreadWorkQueue {
    Thread thread;
    Arena *arena;
    WorkQueue *parent_queue;

    Mutex queue_mutex;
    ConditionVariable task_available_or_finished_cond_var;
    wq__Task *first_free_task;

    struct {
        wq__Task *first;
        wq__Task *last;
        usize list_node_count;
    } queued_tasks;
};

static void wq__alloc_free_tasks(wq__ThreadWorkQueue *queue)
{
    static bool first = true;

    if (!first) {
        // printf("allocating new free tasks in %p\n", (void *)queue);
    } else {
        first = false;

    }

    ASSERT(queue->arena);

    wq__Task *free_tasks = arena_alloc_array(queue->arena, wq__Task, WQ_INITIAL_FREE_TASK_CAP);

    for (s32 i = 0; i < WQ_INITIAL_FREE_TASK_CAP; ++i) {
        wq__Task *task = &free_tasks[i];
        task->next = queue->first_free_task;
        queue->first_free_task = task;
    }
}

static wq__ThreadWorkQueue *wq__get_random_thread_queue(WorkQueue *wq)
{
    s32 index = rng_s32(0, wq->thread_count);

    wq__ThreadWorkQueue *result = &wq->thread_queues[index];

    return result;
}

static wq__Task *wq__pop_queued_task(wq__ThreadWorkQueue *queue)
{
    // NOTE: this should only be called when already holding mutex.
    wq__Task *result = queue->queued_tasks.first;
    queue_pop(&queue->queued_tasks);

    return result;
}

static wq__Task *wq__try_steal_task(WorkQueue *wq, wq__ThreadWorkQueue *self)
{
    ASSERT(self);

    wq__Task *result = 0;

    wq__ThreadWorkQueue *steal_queue = wq__get_random_thread_queue(wq);

    if (steal_queue != self) {
        // printf("No work left, stealing from %p...\n", (void *)steal_queue);
        ScopedLock lock = scoped_lock(steal_queue->queue_mutex);
        result = wq__pop_queued_task(steal_queue);

        if (result) {
            // printf("Successfully stole task %p from %p\n", (void *)result, (void *)steal_queue);
        } else {
            // printf("Failed to steal task\n");
        }
    }

    return result;
}

static wq__Task *wq__task_from_id(TaskID id)
{
    wq__Task *result = (wq__Task *)id.data;

    return result;
}

void wq_run_task(TaskID id)
{
    wq__Task *task = wq__task_from_id(id);
    wq__ThreadWorkQueue *queue = task->from_queue;
    WorkQueue *wq = queue->parent_queue;

    {
        ScopedLock lock = scoped_lock(wq->active_task_count_mutex);
        ++wq->active_task_count;
    }

    #if DEBUG_BUILD
    {
        s32 expect = 0;
        ASSERT(atomic_compare_exchange_s32(&task->has_been_run, &expect, 1)
            && "Ran same task twice");

        {
            ASSERT((atomic_load_s32(&task->generation) == id.generation) && "Stale task provided");
        }
    }
    #endif

    {
        ScopedLock lock = scoped_lock(queue->queue_mutex);
        queue_push(&queue->queued_tasks, task);

        cond_var_signal(queue->task_available_or_finished_cond_var);
    }

}

static wq__Task *wq__try_get_task(wq__ThreadWorkQueue *queue)
{
    wq__Task *result = 0;

    WorkQueue *wq = queue->parent_queue;

    {
        // We only lock the mutex while modifying our queue, as other threads may want to steal
        // from us while we run a task. Also, if we happen to steal from some other queue, we
        // would otherwise hold two queue locks at the same time which is a potential order inversion.
        ScopedLock lock = scoped_lock(queue->queue_mutex);
        result = wq__pop_queued_task(queue);
    }

    if (!result) {
        result = wq__try_steal_task(wq, queue);
    }

    return result;
}

TaskID wq__id_from_task(wq__Task *task)
{
    TaskID result = {};
    result.data = task;
    result.generation = task->generation;

    return result;
}

static void wq__finish_task(wq__Task *task)
{
    wq__ThreadWorkQueue *queue = task->from_queue;

    // Acquire the lock before decrementing the task count. Threads that are waiting for a task to
    // finish will acquire the mutex, check the task count, and then go to sleep if the count is not
    // 0. If we don't lock the mutex before decrementing, we might change the task count inbetween
    // the waiting thread checking the count and going to sleep.

    // TODO: do we need to lock two separate times or can it all be done in one go? We don't want to
    // keep the lock when calling wq__finish_task, as we might then lock another threads mutex,
    // which is a potential order inversion. But we might be able to call wq__finish_task AFTER
    // marking this one as finished, or could that be a problem?
    s32 new_unfinished_count = 0;
    {
        ScopedLock lock = scoped_lock(queue->queue_mutex);

        // TODO: do we even need for this variable to be atomic anymore?
        new_unfinished_count = atomic_dec_s32(&task->unfinished_task_count);
        ASSERT(new_unfinished_count >= 0);
    }

    if (new_unfinished_count == 0) {
        if (task->parent) {
            wq__finish_task(task->parent);
        }

        {
            ScopedLock lock = scoped_lock(queue->queue_mutex);
            atomic_inc_s32(&task->generation);

            // Signal to any threads waiting for the task to finish. This will also wake up any threads
            // waiting for work to do, but as threads in wq_wait_for_task will check for work while
            // waiting for a specific task to finish, this is necessary.
            cond_var_broadcast(queue->task_available_or_finished_cond_var);

            task->next = queue->first_free_task;
            queue->first_free_task = task;
        }
    }
}

static void wq__execute_task(wq__Task *task)
{
    TaskID id = wq__id_from_task(task);
    WorkQueue *wq = task->from_queue->parent_queue;

    task->function(id, task->user_data);
    wq__finish_task(task);

    {
        ScopedLock lock = scoped_lock(wq->active_task_count_mutex);
        --wq->active_task_count;
        cond_var_broadcast(wq->task_finished_cond_var);
    }
}

static b32 wq__task_is_finished(TaskID id, wq__Task *task)
{
    b32 result = atomic_load_s32(&task->generation) != id.generation;

    return result;
}

static wq__ThreadWorkQueue *wq__get_private_work_queue()
{
    wq__ThreadWorkQueue *result = thread_ctx_get()->private_work_queue;

    return result;
}

void wq_wait_for_task(TaskID id)
{
    wq__Task *task = wq__task_from_id(id);
    wq__ThreadWorkQueue *task_queue = task->from_queue;

    // If this thread is a member of a thread queue, it will try to work on its own tasks first.
    wq__ThreadWorkQueue *private_queue = wq__get_private_work_queue();

    // If this thread is not a member of a thread queue, it will simply pick a thread queue at random to focus on.
    if (!private_queue) {
        private_queue = wq__get_random_thread_queue(task_queue->parent_queue);
    }

    while (!wq__task_is_finished(id, task)) {
        wq__Task *task_to_do_while_waiting = wq__try_get_task(private_queue);

        if (task_to_do_while_waiting && !wq__task_is_finished(id, task)) {
            wq__execute_task(task_to_do_while_waiting);
        } else {
            ScopedLock lock = scoped_lock(task_queue->queue_mutex);

            if (!wq__task_is_finished(id, task)) {
                // We wait with a timeout here, since if we are waiting for a task from within another task,
                // if some other task gets pushed to our queue, there is a risk it will never be executed.
                // By waking up periodically to check our own queue, we avoid that problem.
                cond_var_wait_timed(
                    task_queue->task_available_or_finished_cond_var,
                    task_queue->queue_mutex,
                    WQ_WAIT_FOR_TASK_TIMEOUT_IN_USEC);
            }
        }
    }
}

static void *wq__thread_loop(void *user_data)
{
    wq__ThreadWorkQueue *queue = (wq__ThreadWorkQueue *)user_data;

    thread_ctx_get()->private_work_queue = queue;

    WorkQueue *wq = queue->parent_queue;

    while (!atomic_load_s32(&wq->should_terminate)) {
        wq__Task *task = wq__try_get_task(queue);

        if (task && !atomic_load_s32(&wq->should_terminate)) {
            wq__execute_task(task);
        } else {
            // We lock the mutex before checking the should_terminate flag again, as if
            // wq_destroy has been called, we can be sure that then we either see the flag being
            // set, or we wake up from the condition variable broadcast, since the other thread will
            // lock the same mutex before braodcasting.
            // We also need to check if any new tasks have arrived since the last time we
            // checked. If we don't, we will miss the signal on the condition variable and remain
            // asleep.
            ScopedLock lock = scoped_lock(queue->queue_mutex);

            if (!atomic_load_s32(&wq->should_terminate) && list_is_empty(&queue->queued_tasks)) {
                // TODO: should we use timed wait here too?
                cond_var_wait(queue->task_available_or_finished_cond_var, queue->queue_mutex);
            }
        }
    }

    return 0;
}

static void wq__init_thread_work_queue(WorkQueue *wq, wq__ThreadWorkQueue *queue)
{
    queue->arena = arena_create(WQ_THREAD_WORK_QUEUE_ARENA_SIZE);
    queue->queue_mutex = mutex_create();
    queue->task_available_or_finished_cond_var = cond_var_create();
    queue->parent_queue = wq;

    wq__alloc_free_tasks(queue);
}

static void wq__destroy_thread_work_queue(wq__ThreadWorkQueue *queue)
{
    mutex_destroy(queue->queue_mutex);
    cond_var_destroy(queue->task_available_or_finished_cond_var);

    arena_destroy(queue->arena);
}

WorkQueue *wq_create(Arena *arena, s32 thread_count)
{
    WorkQueue *result = arena_alloc_struct(arena, WorkQueue);
    result->thread_queues = arena_alloc_array(arena, wq__ThreadWorkQueue, s32_to_usize(thread_count));
    result->thread_count = thread_count;

    result->active_task_count_mutex = mutex_create();
    result->task_finished_cond_var = cond_var_create();

    for (s32 i = 0; i < thread_count; ++i) {
        wq__ThreadWorkQueue *queue = &result->thread_queues[i];
        wq__init_thread_work_queue(result, queue);
    }

    for (s32 i = 0; i < thread_count; ++i) {
        wq__ThreadWorkQueue *queue = &result->thread_queues[i];
        queue->thread = thread_create(wq__thread_loop, queue);
    }

    return result;
}

void wq_destroy(WorkQueue *wq)
{
    atomic_store_s32(&wq->should_terminate, true);

    // Join threads before destroying resources in case any of the threads are using them.
    for (s32 i = 0; i < wq->thread_count; ++i) {
        wq__ThreadWorkQueue *queue = &wq->thread_queues[i];
        // We'll lock the mutex before broadcasting, since then we can be sure that the worker
        // thread is either waiting on the condition variable and will therefore wake up, or it has
        // seen (or will see) the flag being set.
        {
            ScopedLock lock = scoped_lock(queue->queue_mutex);
            cond_var_broadcast(queue->task_available_or_finished_cond_var);
        }

        thread_join(queue->thread);
    }

    mutex_destroy(wq->active_task_count_mutex);
    cond_var_destroy(wq->task_finished_cond_var);

    for (s32 i = 0; i < wq->thread_count; ++i) {
        wq__ThreadWorkQueue *queue = &wq->thread_queues[i];

        wq__destroy_thread_work_queue(queue);
    }
}

TaskID wq__create_task_impl(WorkQueue *wq, TaskFunction fn, TaskID parent, const void *data, usize size)
{
    wq__ThreadWorkQueue *queue = wq__get_random_thread_queue(wq);
    wq__Task *task = 0;

    {
        ScopedLock lock = scoped_lock(queue->queue_mutex);

        if (!queue->first_free_task) {
            wq__alloc_free_tasks(queue);
        }

        task = queue->first_free_task;
        queue->first_free_task = task->next;
    }

    task->function = fn;
    task->from_queue = queue;
    task->unfinished_task_count = 1;
    task->next = 0;
    task->parent = wq__task_from_id(parent);

    #if DEBUG_BUILD
    {
        atomic_store_s32(&task->has_been_run, false);
    }
    #endif

    if (task->parent) {
        #if DEBUG_BUILD
        {
            ASSERT((atomic_load_s32(&task->parent->generation) == parent.generation)
                && "Provided finished task as parent. Only non-started, or started"
                " but not yet finished tasks are allowed as parents.");
        }
        #endif

        ScopedLock lock = scoped_lock(task->parent->from_queue->queue_mutex);
        atomic_inc_s32(&task->parent->unfinished_task_count);
    }

    if (data) {
        memcpy(task->user_data, data, size);
    }

    TaskID result = wq__id_from_task(task);

    return result;
}

TaskID wq__push_task_impl(WorkQueue *wq, TaskFunction fn, TaskID parent, const void *data, usize size)
{
    TaskID id = wq__create_task_impl(wq, fn, parent, data, size);
    wq_run_task(id);

    return id;
}

void wq_null_function(TaskID, void *)
{
}

usize wq_get_active_task_count(WorkQueue *wq)
{
    ScopedLock lock = scoped_lock(wq->active_task_count_mutex);
    usize result = wq->active_task_count;

    return result;
}

void wq_wait_for_all_tasks(WorkQueue *wq)
{
    ScopedLock lock = scoped_lock(wq->active_task_count_mutex);

    // TODO: try to do tasks in the meantime
    while (wq->active_task_count != 0) {
        cond_var_wait(wq->task_finished_cond_var, wq->active_task_count_mutex);
    }
}

usize wq_get_thread_count(WorkQueue *wq)
{
    usize result = (usize)wq->thread_count;

    return result;
}

usize wq_get_thread_index()
{
    wq__ThreadWorkQueue *private_queue = thread_ctx_get()->private_work_queue;
    ASSERT(private_queue && "Called wq_get_thread_index() on a thread that is not member of a work queue");

    WorkQueue *wq = private_queue->parent_queue;

    ASSERT((private_queue >= wq->thread_queues) && (private_queue < (wq->thread_queues + wq->thread_count)));

    usize result = ssize_to_usize(private_queue - wq->thread_queues);

    return result;
}
