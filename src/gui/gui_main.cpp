#define RESULT_TABLE_SIZE_FRACTION {ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y * 0.8f}

static void gui__begin_frame(gui_State *gui)
{
    (void)gui;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void gui__end_frame(gui_State *gui)
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

static void gui__begin_root_window(gui_State *gui)
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

static void gui__end_window(gui_State *gui)
{
    ImGui::End();
}

b32 gui_should_close(gui_State *gui)
{
    b32 result = glfwWindowShouldClose(gui->window);

    return result;
}

static void gui__main_window(gui_State *gui, app_State *app)
{
    gui__begin_root_window(gui); {
        if (memmi_process_is_null(app->open_process_handle)) {
            ImGui::TextUnformatted("Open process: none");
        } else {
            memmi_PID open_pid = app->open_process_handle.pid;
            char *open_pid_text = str_format_cstr(app->frame_arena, "Open process: %" PRId64, open_pid.value);
            ImGui::TextUnformatted(open_pid_text);
        }

        gui_open_process_button(gui, app);
        gui_open_process_popup(gui, app);

        // NOTE: widgets wrapped in functions should never themselves call
        // BeginChild, we should only do that from top level like this
        {
            char *result_count_label = str_format_cstr(
                app->frame_arena,
                "Results: %zu",
                app->current_result_list.total_match_count);

            ImGui::TextUnformatted(result_count_label);

            ImGui::BeginChild("##child_result_table", RESULT_TABLE_SIZE_FRACTION, 0, 0); {
                gui_scan_result_table(gui, app);
            }; ImGui::EndChild();
        }

        ImGui::SameLine();

        s32 scan_child_flags = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders;

        ImGui::BeginChild("##child_scan", {-FLT_MIN, -FLT_MIN}, scan_child_flags, 0); {
            gui_scan_parameters(gui, app);
        } ImGui::EndChild();
    } gui__end_window(gui);
}

void gui_update_and_render(gui_State *gui, app_State *app)
{
    glfwPollEvents();

    if (glfwGetWindowAttrib(gui->window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
    } else {
        gui__begin_frame(gui);

        bool show_demo_window = false;

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        } else {
            gui__main_window(gui, app);
        }

        gui__end_frame(gui);
    }
}
