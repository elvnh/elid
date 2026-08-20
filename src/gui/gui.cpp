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

#define GUI_OPEN_PROC_POPUP_NAME "Open process"

static void gui__do_open_process_button(Gui *gui, app_State *app, app_CommandList *commands)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_command_push(app->frame_arena, commands, APP_COMMAND_GET_RUNNING_PROCS);

        gui->process_list_popup.selected_index = -1;
        ImGui::OpenPopup(GUI_OPEN_PROC_POPUP_NAME);
    }
}

static void gui__do_open_process_popup(Gui *gui, app_State *app, app_CommandList *commands)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    if (ImGui::BeginPopupModal(GUI_OPEN_PROC_POPUP_NAME, 0, popup_flags)) {
        s32 process_count = usize_to_s32(app->running_process_list.count);

        ImVec2 listbox_size = {-FLT_MIN, -ImGui::GetFrameHeight()};

        if (ImGui::BeginListBox("##process_list", listbox_size)) {
            for (s32 i = 0; i < process_count; ++i) {
                memmi_ProcessInfo proc = app->running_process_list.data[i];
                String proc_name = {(u8 *)proc.name.data, proc.name.count};
                char *proc_name_nt = str_to_cstr(app->frame_arena, proc_name);

                ASSERT(proc_name.count > 0);

                ImGui::PushID(s64_to_s32(proc.pid.value));

                b32 is_selected = gui->process_list_popup.selected_index == i;

                if (ImGui::Selectable(proc_name_nt, is_selected)) {
                    gui->process_list_popup.selected_index = i;
                }

                ImGui::PopID();
            }

            ImGui::EndListBox();
        }

        // TODO: can selected index be a local variable?
        s32 selected_index = gui->process_list_popup.selected_index;
        b32 is_valid_index = (selected_index >= 0) && (selected_index < process_count);

        // TODO: allow double clicking list item
        b32 should_close = false;

        if (ImGui::Button("Open") && is_valid_index) {
            memmi_ProcessInfo clicked_process = app->running_process_list.data[selected_index];

            app_Command *open_proc_command = app_command_push(app->frame_arena, commands, APP_COMMAND_OPEN_PROCESS);
            open_proc_command->as.open_process.pid = clicked_process.pid;

            should_close = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            should_close = true;
        }

        if (should_close) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

app_CommandList gui_update_and_render(Gui *gui, app_State *app)
{
    app_CommandList result = {};

    glfwPollEvents();

    if (glfwGetWindowAttrib(gui->window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
    } else {
        gui__begin_frame(gui);

        bool show_demo_window = false;

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        } else {
            gui__begin_root_window(gui); {
                if (memmi_process_is_null(app->open_process_handle)) {
                    ImGui::Text("Open process: none");
                } else {
                    memmi_PID open_pid = app->open_process_handle.pid;
                    ImGui::Text("Open process: %" PRId64, open_pid.value);
                }

                gui__do_open_process_button(gui, app, &result);
                gui__do_open_process_popup(gui, app, &result);
            } gui__end_window(gui);
        }



        gui__end_frame(gui);
    }

    return result;
}
