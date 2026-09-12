#pragma once

/* TODO:
   - Parent tasks
   - Terminating all threads
   - The queue that the task is pushed to should be notified first
   - Signal cond var before or after releasing lock?
   - Task functions should have void return type
   - Clean up locks, lock in calling function rather than callee
   - Thread local queues so that any thread can start doing work, not only those
     that run the worker thread loop. This way, any thread can do work while waiting for
     a task to finish.
   - Use scoped mutex locks
 */

struct app__ThreadWorkQueue;

typedef void (*app_TaskFunction)(void *user_data);
typedef u32 app_TaskGeneration;

struct app_TaskID {
    uintptr_t id;
    app_TaskGeneration generation;
};

struct app_WorkQueue {
    app__ThreadWorkQueue *thread_queues;
    s32 thread_count;
};

// Can be provided as argument when creating task if no input to task function is needed.
global const s32 app_wq_null_arg = 0;

app_WorkQueue *app_wq_create(Arena *arena, s32 thread_count);
#define        app_wq_create_task(wq, fn, data) app_wq_create_task_impl((wq), (fn), &(data), sizeof((data)))
app_TaskID     app_wq_create_task_impl(app_WorkQueue *wq, app_TaskFunction fn, const void *data, usize data_size);
void           app_wq_run_task(app_WorkQueue *wq, app_TaskID id);
void           app_wq_wait_for_task(app_WorkQueue *wq, app_TaskID id);
