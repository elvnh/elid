#include "gui_state.h"

b32 gui_init(gui_State *gui)
{
    b32 result = false;

    if (glfwInit()) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_RESIZABLE, false);

        f32 main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

        s32 width = 1280;
        s32 height = 800;
        s32 scaled_width = (s32)((f32)width * main_scale);
        s32 scaled_height = (s32)((f32)height * main_scale);

        gui->window = glfwCreateWindow(scaled_width, scaled_height, "elid", 0, 0);

        if (gui->window) {
            glfwMakeContextCurrent(gui->window);
            glfwSwapInterval(1);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO &io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImGui::StyleColorsDark();

            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(main_scale);
            style.FontScaleDpi = main_scale;

            const char *glsl_version = nullptr;
            ImGui_ImplGlfw_InitForOpenGL(gui->window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);

            result = true;
        }
    }

    return result;
}

