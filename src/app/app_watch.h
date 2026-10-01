#pragma once

struct app_WatchEvent {
    uintptr_t instruction_pointer_after_execution;

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
    uintptr_t             start_address;
    u32                   opcode_size;
    u32    				  trigger_count;
    String 				  mnemonic;

    app_WatchInstruction *next;
    app_WatchInstruction *prev;
};

struct app_Watch {
    b32 is_active;
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
