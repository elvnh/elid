#pragma once

enum app_ScannerState : s32 {
    APP_SCANNER_STATE_FIRST_SCAN_READY,
    APP_SCANNER_STATE_NEXT_SCAN_READY,
    APP_SCANNER_STATE_SWITCHED_PROCESS_AFTER_FIRST_SCAN,
};

struct app_State {
    // TODO: pack members into anonymous structs, similar to gui_State
    Arena *permanent_arena; // For allocations that live for the entire program.
    Arena *process_list_arena;

    // Array of arenas for storing scan results. Has a capacity equal to the number of threads in
    // multi_frame_work_queue and can be indexed into using wq_get_thread_index().
    Arena **current_per_thread_scan_result_arenas;
    Arena **previous_per_thread_scan_result_arenas;

    WorkQueue *per_frame_work_queue;
    WorkQueue *multi_frame_work_queue;

    app_ScannerState scanner_state;
    b32 scan_in_progress;

    // For tracking scan progress.
    usize bytes_to_scan;
    usize bytes_scanned;

    memmi_Process open_process_handle;

    memmi_ProcessList running_process_list;
    b32 process_list_ready;

    scan_Result result_list;

    app_AddressTable address_table;

    app_ErrorList error_list;

    Pool *watch_pool;
    app_Watch *watch; // TODO: list of watches?
};

b32  app_init(app_State *app);
void app_destroy(app_State *app);
void app_push_error(app_State *app, app_Error error);
b32  app_has_opened_process(app_State *app);
b32  app_result_table_is_stale(app_State *app);
