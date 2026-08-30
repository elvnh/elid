#pragma once

enum app_ScannerState {
    APP_SCANNER_STATE_FIRST_SCAN_READY,
    APP_SCANNER_STATE_NEXT_SCAN_READY,
};

struct app_State {
    Arena *frame_arena;
    Arena *process_list_arena;
    Arena *scan_result_arena;

    app_ScannerState scanner_state;
    memmi_Process open_process_handle;
    memmi_ProcessList running_process_list;
    scan_Result current_result_list;
};

b32  app_init(app_State *app);
