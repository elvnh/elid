#pragma once

struct app_State {
    Arena *frame_arena;
    Arena *process_list_arena;
    Arena *scan_result_arena;

    memmi_Process open_process_handle;
    memmi_ProcessList running_process_list;
    scan_Result current_result_list;
};

b32  app_init(app_State *app);
