#include "app_work_queue.h"

#define APP_WQ_TASK_USER_DATA_SIZE KB(4)
#define APP_WQ_INITIAL_FREE_TASK_CAP 16
#define APP_WQ_THREAD_WORK_QUEUE_ARENA_SIZE APP_WQ_INITIAL_FREE_TASK_CAP * sizeof(app__Task)

struct app__Task {
    char user_data[APP_WQ_TASK_USER_DATA_SIZE];

    app_TaskFunction function;
    app__Task *parent;
    app__ThreadWorkQueue *from_queue;
    s32 unfinished_task_count;

    Mutex task_finish_mutex;
    ConditionVariable task_finish_cond_var;
    app_TaskGeneration generation; // Increased when a task is finished

    app__Task *next;
};

struct app__ThreadWorkQueue {
    Arena *arena;
    app_WorkQueue *parent_queue;

    Mutex queue_mutex;
    ConditionVariable task_available_cond_var;
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
        printf("allocating new free tasks in %p\n", (void *)queue);
    } else {
        first = false;

    }

    ASSERT(queue->arena);

    app__Task *free_tasks = arena_alloc_array(queue->arena, app__Task, APP_WQ_INITIAL_FREE_TASK_CAP);

    for (s32 i = 0; i < APP_WQ_INITIAL_FREE_TASK_CAP; ++i) {
        app__Task *task = &free_tasks[i];
        task->next = queue->first_free_task;
        queue->first_free_task = task;

        task->task_finish_mutex = mutex_create();
        task->task_finish_cond_var = cond_var_create();
    }
}

static app__ThreadWorkQueue *app__wq_get_random_thread_queue(app_WorkQueue *wq)
{
    s32 index = 0; // TODO: actual randomness
    app__ThreadWorkQueue *result = &wq->thread_queues[index];

    return result;
}

static app__Task *app__wq_pop_queued_task(app__ThreadWorkQueue *queue)
{
    // TODO: don't lock here, lock in calling function
    mutex_lock(queue->queue_mutex);

    app__Task *result = queue->queued_tasks.first;
    queue_pop(&queue->queued_tasks);

    mutex_unlock(queue->queue_mutex);

    return result;
}

static app__Task *app__wq_get_next_task(app_WorkQueue *wq, app__ThreadWorkQueue *queue)
{
    app__Task *result = 0;

    if (!queue) {
        // This branch will be entered when a thread that waits for a task grabs
        // some other task to do while waiting.
        // TODO: make it so that each thread has its own thread local queue
        ASSERT(0);
        queue = app__wq_get_random_thread_queue(wq);
    }

    result = app__wq_pop_queued_task(queue);

    if (!result) {
        app__ThreadWorkQueue *steal_queue = app__wq_get_random_thread_queue(wq);

        if (steal_queue != queue) {
            // printf("No work left, stealing from %p...\n", (void *)steal_queue);
            result = app__wq_pop_queued_task(steal_queue);

            if (result) {
                printf("Successfully stole task %p from %p\n", (void *)result, (void *)steal_queue);
            } else {
                // printf("Failed to steal task\n");
            }
        }
    }

    return result;
}

static app__Task *app__wq_task_from_id(app_TaskID id)
{
    app__Task *result = (app__Task *)id.id;

    return result;
}

void app_wq_run_task(app_WorkQueue *wq, app_TaskID id)
{
    app__Task *task = app__wq_task_from_id(id);

    mutex_lock(task->task_finish_mutex);
    ASSERT((task->generation == id.generation) && "Stale task provided");
    mutex_unlock(task->task_finish_mutex);

    app__ThreadWorkQueue *queue = task->from_queue;

    mutex_lock(queue->queue_mutex);
    queue_push(&queue->queued_tasks, task);
    mutex_unlock(queue->queue_mutex);

    cond_var_signal(queue->task_available_cond_var);
}

// TODO: app__wq_ -> app_wq__

void app_wq_wait_for_task(app_WorkQueue *wq, app_TaskID id)
{
    app__Task *task = app__wq_task_from_id(id);

    mutex_lock(task->task_finish_mutex);

    // We only need to wait if this ID has the same generation as the task. If
    // it doesn't, the task has already finished and we don't need to wait.
    while (task->generation == id.generation) {
        cond_var_wait(task->task_finish_cond_var, task->task_finish_mutex);
    }

    mutex_unlock(task->task_finish_mutex);
}

static void app__wq_finish_task(app__Task *task)
{
    s32 new_unfinished_count = atomic_dec_s32(&task->unfinished_task_count);
    ASSERT(new_unfinished_count >= 0);

    if (new_unfinished_count == 0) {
        printf("unfinished count reached 0, finishing task...\n");

        {
            mutex_lock(task->task_finish_mutex);
            ++task->generation;
            mutex_unlock(task->task_finish_mutex);
        }

        // Signal to any threads waiting for the task to finish.
        cond_var_broadcast(task->task_finish_cond_var);

        if (task->parent) {
            printf("recursively finishing parent task...\n");
            app__wq_finish_task(task->parent);
        }

        app__ThreadWorkQueue *queue = task->from_queue;
        mutex_lock(queue->queue_mutex);

        #if DEBUG_BUILD
        for (app__Task *t = list_head(&queue->queued_tasks); t; t = list_next(t)) {
            if (t == task) {
                ASSERT(0 && "Task exists as a task in flight and a queued task at the same time");
            }
        }
        #endif

        task->next = queue->first_free_task;
        queue->first_free_task = task;

        mutex_unlock(queue->queue_mutex);
    }
}

static void app__wq_execute_task(app__Task *task)
{
    task->function(task->user_data);
    app__wq_finish_task(task);
}

static void *app__wq_thread_loop(void *user_data)
{
    app__ThreadWorkQueue *queue = (app__ThreadWorkQueue *)user_data;

    mutex_lock(queue->queue_mutex);

    while (true) {
        app__Task *task = app__wq_get_next_task(queue->parent_queue, queue);

        if (task) {
            printf("grabbing task %p...\n", (void *)task);
            app__wq_execute_task(task);
        } else {
            printf("Failed to get task, sleeping...\n");
            cond_var_wait(queue->task_available_cond_var, queue->queue_mutex);
        }
    }

    mutex_unlock(queue->queue_mutex);

    return 0;
}

static void app__wq_init_thread_work_queue(app_WorkQueue *wq, app__ThreadWorkQueue *queue)
{
    queue->arena = arena_create(APP_WQ_THREAD_WORK_QUEUE_ARENA_SIZE);
    queue->queue_mutex = mutex_create();
    queue->task_available_cond_var = cond_var_create();
    queue->parent_queue = wq;

    app__wq_alloc_free_tasks(queue);
}

app_WorkQueue *app_wq_create(Arena *arena, s32 thread_count)
{
    app_WorkQueue *result = arena_alloc_struct(arena, app_WorkQueue);
    result->thread_queues = arena_alloc_array(arena, app__ThreadWorkQueue, s32_to_usize(thread_count));
    result->thread_count = thread_count;

    for (s32 i = 0; i < thread_count; ++i) {
        app__ThreadWorkQueue *queue = &result->thread_queues[i];
        app__wq_init_thread_work_queue(result, queue);
    }

    for (s32 i = 0; i < thread_count; ++i) {
        thread_create(app__wq_thread_loop, &result->thread_queues[i]);
    }

    return result;
}

app_TaskID app_wq_create_task_impl(app_WorkQueue *wq, app_TaskFunction fn, app_TaskID parent, const void *data, usize size)
{
    app__ThreadWorkQueue *queue = app__wq_get_random_thread_queue(wq);

    // TODO: scoped lock
    mutex_lock(queue->queue_mutex);

    if (!queue->first_free_task) {
        app__wq_alloc_free_tasks(queue);
    }

    app__Task *task = queue->first_free_task;
    queue->first_free_task = task->next;

    mutex_unlock(queue->queue_mutex);

    task->function = fn;
    task->from_queue = queue;
    task->unfinished_task_count = 1;
    task->next = 0;
    task->parent = app__wq_task_from_id(parent);

    if (task->parent) {
        atomic_inc_s32(&task->parent->unfinished_task_count);

        #if DEBUG_BUILD
        // Check that user didn't provide an already finished task as parent
        mutex_lock(task->parent->task_finish_mutex);

        ASSERT((task->parent->generation == parent.generation) && "Provided finished task as parent");

        mutex_unlock(task->parent->task_finish_mutex);
        #endif
    }

    if (data) {
        memcpy(task->user_data, data, size);
    }

    app_TaskID result = {};
    result.id = (uintptr_t)task;
    result.generation = task->generation;

    return result;
}
