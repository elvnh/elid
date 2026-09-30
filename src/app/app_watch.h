#pragma once

struct app_WatchEvent {
    uintptr_t address;

    app_WatchEvent *next;
    app_WatchEvent *prev;
};

struct app_Watch {
    memmi_Process process;

    Mutex list_mutex;

    // TODO: use pool instead
    Arena *event_arena;
    struct {
        app_WatchEvent *first;
        app_WatchEvent *last;
        usize list_node_count;
    } event_queue;

    b32 should_terminate; // Atomic
    u32 breakpoint_index;
    app_TaskID task;
};

typedef enum {
    APP_WATCH_COND_WRITE,
    APP_WATCH_COND_READ_WRITE,
} app_WatchCondition;

app_Watch      *app_watch_begin(app_WorkQueue *wq, memmi_Process proc, uintptr_t address, app_WatchCondition cond, u32 size, u32 index);
void            app_watch_destroy(app_Watch *watch);
app_WatchEvent *app_watch_pop_event(app_Watch *watch);
