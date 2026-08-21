#pragma once

struct app_State {
    Arena *frame_arena;
    Arena *process_list_arena;

    memmi_Process open_process_handle;
    memmi_ProcessList running_process_list;
};

b32  app_init(app_State *app);
void app_get_running_processes(app_State *app);
b32  app_open_process(app_State *app, memmi_PID pid);
