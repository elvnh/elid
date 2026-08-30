#include "app_actions.h"

void app_action_get_running_processes(app_State *app)
{
    arena_reset(app->process_list_arena);

    memmi_Allocator allocator = proc_memmi_allocator_from_arena(app->process_list_arena);

    // TODO: do this async, and display empty window in meantime?
    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;
}

b32 app_action_open_process(app_State *app, memmi_PID pid)
{
    b32 result = false;

    memmi_OpenProcess open_process_result = memmi_open_process(pid);

    if (open_process_result.status == MEMMI_OK) {
        app->open_process_handle = open_process_result.process;
        result = true;
    }

    return result;

}

void app_action_run_first_scan(app_State *app, scan_Comparison cmp, usize stride)
{
    // TODO: async

    app->current_result_list = scan_remote_memory_first(
        app->scan_result_arena, app->open_process_handle, cmp, stride);
}
