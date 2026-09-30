#pragma once

struct app_WatchEvent {
    uintptr_t rip_register;

    app_WatchEvent *next;
    app_WatchEvent *prev;
};

struct app_WatchThreadContext {
    memmi_Process process;

    struct {
        Mutex mutex;
        Pool *pool;

        app_WatchEvent *first;
        app_WatchEvent *last;
        usize list_node_count;
    } event_queue;

    b32 should_terminate; // Atomic
    u32 breakpoint_index;
}; 

struct app_WatchInstruction {
    uintptr_t address;

    // TODO: opcode, mnemonic etc.

    app_WatchInstruction *next;
    app_WatchInstruction *prev;
};

struct app_Watch {
    proc_Status status;
    Arena *arena;
    app_WatchThreadContext thread_context;
    app_TaskID task;

    struct {
        app_WatchInstruction *first;
        app_WatchInstruction *last;
        usize list_node_count;
    } triggered_instruction_list;
};

typedef enum {
    APP_WATCH_COND_WRITE,
    APP_WATCH_COND_READ_WRITE,
} app_WatchCondition;

app_Watch app_watch_begin(app_WorkQueue *wq, memmi_Process proc, uintptr_t address, app_WatchCondition cond, u32 size, u32 index);
void      app_watch_destroy(app_Watch *watch);
void      app_watch_poll_events(app_Watch *watch);
