#pragma once

struct app_WatchEvent {
    uintptr_t instruction_pointer_after_execution;

    app_WatchEvent *next;
    app_WatchEvent *prev;
};

struct app_WatchInstruction {
    uintptr_t             start_address;
    u32                   opcode_size;
    u32    				  trigger_count;
    String 				  mnemonic;

    app_WatchInstruction *next;
    app_WatchInstruction *prev;
};

typedef enum {
    APP_WATCH_COND_WRITE,
    APP_WATCH_COND_READ_WRITE,
} app_WatchCondition;

struct app_Watch {
    Arena *arena;

    memmi_Process process;
    uintptr_t breakpoint_address;
    u32 breakpoint_index;
    app_WatchCondition breakpoint_condition;
    u32 breakpoint_length;

    app_TaskID task; // The watch task handle.

    // Asynchronously filled by the watch thread.
    struct {
        Mutex mutex;
        Pool *pool;

        app_WatchEvent *first;
        app_WatchEvent *last;
        usize list_node_count;
    } event_queue;

    b32 should_terminate; // Atomic, indicates whether the watch thread should stop.

    // Filled out at the start of each frame by polling for new events.
    struct {
        app_WatchInstruction *first;
        app_WatchInstruction *last;
        usize list_node_count;
    } triggered_instruction_list;
};

void        app_watch_init(app_Watch *watch, memmi_Process proc, uintptr_t address,
                           u32 breakpoint_index, app_WatchCondition cond, u32 breakpoint_length);
void        app_watch_run(app_Watch *watch);
proc_Status app_watch_poll_events(app_Watch *watch);
void        app_watch_destroy(app_Watch *watch);
