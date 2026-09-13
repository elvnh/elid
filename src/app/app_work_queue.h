#pragma once

/* TODO:
   - Thread local queues so that any thread can start doing work, not only those
     that run the worker thread loop. This way, any thread can do work while waiting for
     a task to finish.
   - Logging at which source location a task was created
 */

struct app__ThreadWorkQueue;
typedef u32 app_TaskGeneration;

struct app_TaskID {
    void *data;
    app_TaskGeneration generation;
};

typedef void (*app_TaskFunction)(app_TaskID self, void *user_data);

struct app_WorkQueue {
    app__ThreadWorkQueue *thread_queues;
    s32 thread_count;
    b32 should_terminate; // Atomic
};

// Can be provided as argument when creating task if no input to task function is needed.
global const s32 app_wq_null_arg = {};
void app_wq_null_function(app_TaskID, void *);

app_WorkQueue *app_wq_create(Arena *arena, s32 thread_count);
void           app_wq_destroy(app_WorkQueue *wq);
#define        app_wq_create_task(wq, fn, data) app_wq_create_task_impl((wq), (fn), {}, &(data), sizeof((data)))
#define        app_wq_create_task_as_child(wq, fn, data, parent) app_wq_create_task_impl((wq), (fn), (parent), &(data), sizeof((data)))
void           app_wq_run_task(app_TaskID id);
void           app_wq_wait_for_task(app_TaskID id);
