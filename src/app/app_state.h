#pragma once

enum app_ScannerState : s32 {
    APP_SCANNER_STATE_FIRST_SCAN_READY,
    APP_SCANNER_STATE_NEXT_SCAN_READY,
};

struct app_State {
    // TODO: pack members into anonymous structs, similar to gui_State
    Arena *permanent_arena; // For allocations that live for the entire program.
    Arena *process_list_arena;
    Arena *scan_result_arena;
    Arena *previous_scan_result_arena;

    app_WorkQueue *per_frame_work_queue;
    app_WorkQueue *multi_frame_work_queue;

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
