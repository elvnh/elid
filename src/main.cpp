#include <stdio.h>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "base/base_all.cpp"
#include "proc/proc_all.cpp"
#include "scan/scan_all.cpp"
#include "app/app_all.cpp"
#include "gui/gui_all.cpp"

int main(int argc, char **argv)
{
    int result = 0;

    s64 pid_arg = -1;
    b32 pid_parsing_succeeded = true;

    if (argc > 1) {
        pid_parsing_succeeded = stn_parse_s64(argv[1], strlen(argv[1]), &pid_arg, 10);
        if (!pid_parsing_succeeded) {
            fprintf(stderr, "Pid provided was not valid.\n");
        }
    }

    app_State app = {};
    gui_State gui = {};

    if (!app_init(&app) || !gui_init(&gui)) {
        result = 1;
    } else {
        if (pid_parsing_succeeded && (pid_arg != -1)) {
            if (!app_action_open_process(&app, {pid_arg})) {
                fprintf(stderr, "Failed to open provided pid.\n");
            }
        }

        // TODO: move this functionality into app_run
        while (!gui_should_close(&gui)) {
            arena_reset(gui.frame_arena);

            // Dummy task that all per-frame tasks will be children of, so that we can wait for all
            // of them at the end of the frame.
            app_TaskID frame_task = app_wq_create_task(app.work_queue, app_wq_null_function, app_wq_null_arg);
            app.frame_root_task = frame_task;

            gui_update_and_render(&gui, &app);

            // TODO: should wait_for_task run the task if not already started?
            app_wq_run_task(frame_task);
            app_wq_wait_for_task(frame_task);
        }
    }

    return result;
}
