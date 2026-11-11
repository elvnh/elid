#include <stdio.h>
#include <math.h>

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

        gui_run(&gui, &app);

        app_destroy(&app);
    }

    return result;
}
