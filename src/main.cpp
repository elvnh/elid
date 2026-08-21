#include <stdio.h>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "base/base_all.cpp"
#include "scan/scan_all.cpp"
#include "app/app_all.cpp"
#include "gui/gui_all.cpp"

int main()
{


    return 0;

    int result = 0;

    app_State app = {};
    gui_State gui = {};

    if (!app_init(&app) || !gui_init(&gui)) {
        result = 1;
    } else {
        // TODO: move this functionality into app_run
        while (!gui_should_close(&gui)) {
            arena_reset(app.frame_arena);

            gui_update_and_render(&gui, &app);
        }
    }

    return result;
}
