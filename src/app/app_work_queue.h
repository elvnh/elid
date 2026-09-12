#pragma once

/* TODO:
   - Parent tasks
   - Terminating all threads
   - Better API for providing user data
   - The queue that the task is pushed to should be notified first
   - Signal cond var before or after releasing lock?
   - Task functions should have void return type
   - Clean up locks, lock in calling function rather than callee
   - Thread local queues so that any thread can start doing work, not only those
     that run the worker thread loop. This way, any thread can do work while waiting for
     a task to finish.
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

app_WorkQueue *app_wq_create(Arena *arena, s32 thread_count);

app_TaskID app_wq_create_task(app_WorkQueue *wq, app_TaskFunction function, void *user_data, usize user_data_size);
void app_wq_run_task(app_WorkQueue *wq, app_TaskID id);
void app_wq_wait_for_task(app_WorkQueue *wq, app_TaskID id);
