#pragma once

/* TODO:
   - Logging at which source location a task was created
 */

/* struct wq__ThreadWorkQueue; */

typedef u32 TaskGeneration;

// TODO: rename to task
struct TaskID {
    void *data;
    TaskGeneration generation;
};

typedef void (*TaskFunction)(TaskID self, void *user_data);

struct WorkQueue {
    struct wq__ThreadWorkQueue *thread_queues;
    s32 thread_count;
    b32 should_terminate;   // Atomic

    // The lowest of all thread ID's in this work queue. Can be used to use thread ID's
    // as index into an array by subtracting this from a thread ID.
    u64 minimum_thread_id;

    Mutex active_task_count_mutex;
    ConditionVariable task_finished_cond_var;
    // TODO: can this be made atomic instead?
    usize active_task_count;
};

// Can be provided as argument when creating task if no input to task function is needed.
global const s32 wq_null_arg = {};
void wq_null_function(TaskID, void *);

WorkQueue *wq_create(Arena *arena, s32 thread_count);
void       wq_destroy(WorkQueue *wq);
#define    wq_create_task(wq, fn, data) wq_create_task_as_child((wq), (fn), (data), TaskID{})
#define    wq_push_task(wq, fn, data) wq_push_task_as_child((wq), (fn), (data), TaskID{})
#define    wq_create_task_as_child(wq, fn, data, parent) wq__create_task_impl((wq), (fn), (parent), &(data), sizeof((data)))
#define    wq_push_task_as_child(wq, fn, data, parent) wq__push_task_impl((wq), (fn), (parent), &(data), sizeof((data)))
void       wq_run_task(TaskID id);
void       wq_wait_for_task(TaskID id);
void       wq_wait_for_all_tasks(WorkQueue *wq);
usize      wq_get_active_task_count(WorkQueue *wq);
usize      wq_get_thread_count(WorkQueue *wq);
usize      wq_thread_id_to_index();
