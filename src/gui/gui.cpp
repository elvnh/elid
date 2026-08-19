#include "gui.h"

b32 gui_init(Gui *gui)
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

b32 gui_should_close(Gui *gui)
{
    b32 result = glfwWindowShouldClose(gui->window);

    return result;
}

static void gui__begin_frame(Gui *gui)
{
    (void)gui;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void gui__end_frame(Gui *gui)
{
    ImGui::Render();

    int display_width, display_height;
    glfwGetFramebufferSize(gui->window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(gui->window);
}

static void gui__begin_root_window(Gui *gui)
{
    ImGui::SetNextWindowPos(ImVec2{});
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGuiWindowFlags root_window_flags =
        ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoSavedSettings
        //| ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##root_window", 0, root_window_flags);
}

static void gui__end_window(Gui *gui)
{
    ImGui::End();
}

static void gui__do_open_process_button(Gui *gui, app_State *app)
{
    if (ImGui::Button("Open process")) {
        TODO("Open process");
    }
}

void gui_update_and_render(Gui *gui, app_State *app)
{
    glfwPollEvents();

    if (glfwGetWindowAttrib(gui->window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
    } else {
        gui__begin_frame(gui);

        gui__begin_root_window(gui); {
            ImGui::Text("No process opened");

            gui__do_open_process_button(gui, app);

        } gui__end_window(gui);

        gui__end_frame(gui);
    }
}
