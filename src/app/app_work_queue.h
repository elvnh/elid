#pragma once

// TODO: waiting for specific task
// TODO: terminating all threads

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

// TODO: macro that passes size automatically
app_TaskID app_wq_create_task(app_WorkQueue *wq, app_TaskFunction function, void *user_data, usize user_data_size);
void app_wq_run_task(app_WorkQueue *wq, app_TaskID id);
void app_wq_wait_for_task(app_WorkQueue *wq, app_TaskID id);
