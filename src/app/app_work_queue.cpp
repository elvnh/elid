#include "app_work_queue.h"

#define APP_WQ_TASK_USER_DATA_SIZE KB(4)
#define APP_WQ_INITIAL_FREE_TASK_CAP 16
#define APP_WQ_THREAD_WORK_QUEUE_ARENA_SIZE APP_WQ_INITIAL_FREE_TASK_CAP * sizeof(app__Task)

struct app__Task {
    char user_data[APP_WQ_TASK_USER_DATA_SIZE];

    app_TaskFunction function;
    app__Task *parent;
    app__ThreadWorkQueue *from_queue;

    s32 unfinished_task_count; // Atomic, number of unfinished tasks including child tasks
    app_TaskGeneration generation; // Atomic, increased when a task is finished

    app__Task *next; // Either next free task or next enqueued task

    #if DEBUG_BUILD
    b32 has_been_run;
    #endif
};

struct app__ThreadWorkQueue {
    Thread thread;
    Arena *arena;
    app_WorkQueue *parent_queue;

    Mutex queue_mutex;
    ConditionVariable task_available_or_finished_cond_var;
    app__Task *first_free_task;

    struct {
        app__Task *first;
        app__Task *last;
        usize list_node_count;
    } queued_tasks;
};

static void app__wq_alloc_free_tasks(app__ThreadWorkQueue *queue)
{
    static bool first = true;

    if (!first) {
        // printf("allocating new free tasks in %p\n", (void *)queue);
    } else {
        first = false;

    }

    ASSERT(queue->arena);

    app__Task *free_tasks = arena_alloc_array(queue->arena, app__Task, APP_WQ_INITIAL_FREE_TASK_CAP);

    for (s32 i = 0; i < APP_WQ_INITIAL_FREE_TASK_CAP; ++i) {
        app__Task *task = &free_tasks[i];
        task->next = queue->first_free_task;
        queue->first_free_task = task;
    }
}

static app__ThreadWorkQueue *app__wq_get_random_thread_queue(app_WorkQueue *wq)
{
    s32 index = rng_s32(0, wq->thread_count);

    app__ThreadWorkQueue *result = &wq->thread_queues[index];

    return result;
}

static app__Task *app__wq_pop_queued_task(app__ThreadWorkQueue *queue)
{
    // NOTE: this should only be called when already holding mutex.
    app__Task *result = queue->queued_tasks.first;
    queue_pop(&queue->queued_tasks);

    return result;
}

static app__Task *app__wq_try_steal_task(app_WorkQueue *wq, app__ThreadWorkQueue *self)
{
    ASSERT(self);

    app__Task *result = 0;

    app__ThreadWorkQueue *steal_queue = app__wq_get_random_thread_queue(wq);

    if (steal_queue != self) {
        // printf("No work left, stealing from %p...\n", (void *)steal_queue);
        ScopedLock lock = scoped_lock(steal_queue->queue_mutex);
        result = app__wq_pop_queued_task(steal_queue);

        if (result) {
            // printf("Successfully stole task %p from %p\n", (void *)result, (void *)steal_queue);
        } else {
            // printf("Failed to steal task\n");
        }
    }

    return result;
}

static app__Task *app__wq_task_from_id(app_TaskID id)
{
    app__Task *result = (app__Task *)id.data;

    return result;
}

void app_wq_run_task(app_TaskID id)
{
    app__Task *task = app__wq_task_from_id(id);
    app__ThreadWorkQueue *queue = task->from_queue;
    app_WorkQueue *wq = queue->parent_queue;

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

// TODO: app__wq_ -> app_wq__
static app__Task *app__wq_try_get_task(app__ThreadWorkQueue *queue)
{
    app__Task *result = 0;

    app_WorkQueue *wq = queue->parent_queue;

    {
        // We only lock the mutex while modifying our queue, as other threads may want to steal
        // from us while we run a task. Also, if we happen to steal from some other queue, we
        // would otherwise hold two queue locks at the same time which is a potential order inversion.
        ScopedLock lock = scoped_lock(queue->queue_mutex);
        result = app__wq_pop_queued_task(queue);
    }

    if (!result) {
        result = app__wq_try_steal_task(wq, queue);
    }

    return result;
}

app_TaskID app__wq_id_from_task(app__Task *task)
{
    app_TaskID result = {};
    result.data = task;
    result.generation = task->generation;

    return result;
}

static void app__wq_finish_task(app__Task *task)
{
    app__ThreadWorkQueue *queue = task->from_queue;

    s32 new_unfinished_count = atomic_dec_s32(&task->unfinished_task_count);
    ASSERT(new_unfinished_count >= 0);

    if (new_unfinished_count == 0) {
        // printf("unfinished count reached 0, finishing task...\n");
        if (task->parent) {
            // printf("recursively finishing parent task...\n");
            app__wq_finish_task(task->parent);
        }

        {
            ScopedLock lock = scoped_lock(queue->queue_mutex);
            atomic_inc_s32(&task->generation);

            // Signal to any threads waiting for the task to finish. This will also wake up any threads
            // waiting for work to do, but as threads in app_wq_wait_for_task will check for work while
            // waiting for a specific task to finish, this is necessary.
            cond_var_broadcast(queue->task_available_or_finished_cond_var);


            #if DEBUG_BUILD // TODO: remove
            for (app__Task *t = list_head(&queue->queued_tasks); t; t = list_next(t)) {
                if (t == task) {
                    ASSERT(0 && "Task exists as a task in flight and a queued task at the same time");
                }
            }
            #endif

            task->next = queue->first_free_task;
            queue->first_free_task = task;
        }
    }
}

static void app__wq_execute_task(app__Task *task)
{
    app_TaskID id = app__wq_id_from_task(task);
    app_WorkQueue *wq = task->from_queue->parent_queue;

    task->function(id, task->user_data);
    app__wq_finish_task(task);

    {
        ScopedLock lock = scoped_lock(wq->active_task_count_mutex);
        --wq->active_task_count;
        cond_var_broadcast(wq->task_finished_cond_var);
    }
}

static b32 app__wq_task_is_finished(app_TaskID id, app__Task *task)
{
    b32 result = atomic_load_s32(&task->generation) != id.generation;

    return result;
}

void app_wq_wait_for_task(app_TaskID id)
{
    app__Task *task = app__wq_task_from_id(id);
    app__ThreadWorkQueue *queue = task->from_queue;

    while (!app__wq_task_is_finished(id, task)) {
        app__Task *task_to_do_while_waiting = app__wq_try_get_task(queue);

        if (task_to_do_while_waiting && !app__wq_task_is_finished(id, task)) {
            app__wq_execute_task(task_to_do_while_waiting);
        } else {
            // We wait on the queue global cond var as we're both interested in new jobs arriving
            // and the task finishing. We also check the generation again before going to sleep,
            // since it may have changed since last check. It won't change after acquiring lock
            // though, as the lock is always acquired before increasing it.
            ScopedLock lock = scoped_lock(queue->queue_mutex);

            if (!app__wq_task_is_finished(id, task)) {
                cond_var_wait(queue->task_available_or_finished_cond_var, queue->queue_mutex);
            }
        }
    }
}

static void *app__wq_thread_loop(void *user_data)
{
    app__ThreadWorkQueue *queue = (app__ThreadWorkQueue *)user_data;

    app_WorkQueue *wq = queue->parent_queue;

    while (!atomic_load_s32(&wq->should_terminate)) {
        app__Task *task = app__wq_try_get_task(queue);

        if (task && !atomic_load_s32(&wq->should_terminate)) {
            app__wq_execute_task(task);
        } else {
            // We lock the mutex before checking the should_terminate flag again, as if
            // app_wq_destroy has been called, we can be sure that then we either see the flag being
            // set, or we wake up from the condition variable broadcast, since the other thread will
            // lock the same mutex before braodcasting.
            // We also need to check if any new tasks have arrived since the last time we
            // checked. If we don't, we will miss the signal on the condition variable and remain
            // asleep.
            ScopedLock lock = scoped_lock(queue->queue_mutex);

            if (!atomic_load_s32(&wq->should_terminate) && list_is_empty(&queue->queued_tasks)) {
                cond_var_wait(queue->task_available_or_finished_cond_var, queue->queue_mutex);
            }
        }
    }

    return 0;
}

static void app__wq_init_thread_work_queue(app_WorkQueue *wq, app__ThreadWorkQueue *queue)
{
    queue->arena = arena_create(APP_WQ_THREAD_WORK_QUEUE_ARENA_SIZE);
    queue->queue_mutex = mutex_create();
    queue->task_available_or_finished_cond_var = cond_var_create();
    queue->parent_queue = wq;

    app__wq_alloc_free_tasks(queue);
}

static void app__wq_destroy_thread_work_queue(app__ThreadWorkQueue *queue)
{
    mutex_destroy(queue->queue_mutex);
    cond_var_destroy(queue->task_available_or_finished_cond_var);

    arena_destroy(queue->arena);
}

app_WorkQueue *app_wq_create(Arena *arena, s32 thread_count)
{
    app_WorkQueue *result = arena_alloc_struct(arena, app_WorkQueue);
    result->thread_queues = arena_alloc_array(arena, app__ThreadWorkQueue, s32_to_usize(thread_count));
    result->thread_count = thread_count;

    result->active_task_count_mutex = mutex_create();
    result->task_finished_cond_var = cond_var_create();

    for (s32 i = 0; i < thread_count; ++i) {
        app__ThreadWorkQueue *queue = &result->thread_queues[i];
        app__wq_init_thread_work_queue(result, queue);
    }

    for (s32 i = 0; i < thread_count; ++i) {
        app__ThreadWorkQueue *queue = &result->thread_queues[i];
        queue->thread = thread_create(app__wq_thread_loop, queue);
    }

    return result;
}

void app_wq_destroy(app_WorkQueue *wq)
{
    atomic_store_s32(&wq->should_terminate, true);

    // Join threads before destroying resources in case any of the threads are using them.
    for (s32 i = 0; i < wq->thread_count; ++i) {
        app__ThreadWorkQueue *queue = &wq->thread_queues[i];
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
        app__ThreadWorkQueue *queue = &wq->thread_queues[i];

        app__wq_destroy_thread_work_queue(queue);
    }
}

app_TaskID app_wq_create_task_impl(app_WorkQueue *wq, app_TaskFunction fn, app_TaskID parent, const void *data, usize size)
{
    app__ThreadWorkQueue *queue = app__wq_get_random_thread_queue(wq);
    app__Task *task = 0;

    {
        ScopedLock lock = scoped_lock(queue->queue_mutex);

        if (!queue->first_free_task) {
            app__wq_alloc_free_tasks(queue);
        }

        task = queue->first_free_task;
        queue->first_free_task = task->next;
    }

    task->function = fn;
    task->from_queue = queue;
    task->unfinished_task_count = 1;
    task->next = 0;
    task->parent = app__wq_task_from_id(parent);

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

        atomic_inc_s32(&task->parent->unfinished_task_count);
    }

    if (data) {
        memcpy(task->user_data, data, size);
    }

    app_TaskID result = app__wq_id_from_task(task);

    return result;
}

app_TaskID app_wq_push_task_impl(app_WorkQueue *wq, app_TaskFunction fn, app_TaskID parent, const void *data, usize size)
{
    app_TaskID id = app_wq_create_task_impl(wq, fn, parent, data, size);
    app_wq_run_task(id);

    return id;
}

void app_wq_null_function(app_TaskID, void *)
{
}

usize app_wq_get_active_task_count(app_WorkQueue *wq)
{
    ScopedLock lock = scoped_lock(wq->active_task_count_mutex);
    usize result = wq->active_task_count;

    return result;
}

void app_wq_wait_for_all_tasks(app_WorkQueue *wq)
{
    ScopedLock lock = scoped_lock(wq->active_task_count_mutex);

    while (wq->active_task_count != 0) {
        cond_var_wait(wq->task_finished_cond_var, wq->active_task_count_mutex);
    }
}
