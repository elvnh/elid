#include <stdio.h>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "base/base_all.cpp"
#include "app/app_all.cpp"
#include "gui/gui_all.cpp"

int main()
{
    int result = 0;

    app_State app = {};
    Gui gui = {};

    if (!app_init(&app) || !gui_init(&gui)) {
        result = 1;
    } else {
        while (!gui_should_close(&gui)) {
            arena_reset(app.frame_arena);

            app_CommandList commands = gui_update_and_render(&gui, &app);

            app_execute_commands(&app, commands);
        }
    }


    return result;
}
