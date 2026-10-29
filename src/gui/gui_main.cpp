#define RESULT_TABLE_WIDTH_FRACTION (ImGui::GetContentRegionAvail().x * 0.4f)
#define RESULT_TABLE_HEIGHT_FRACTION (ImGui::GetContentRegionAvail().y * 0.7f)
#define RESULT_TABLE_SIZE_FRACTION {RESULT_TABLE_WIDTH_FRACTION, RESULT_TABLE_HEIGHT_FRACTION}

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

    glClearColor(clear_color.x * clear_color.w,
        clear_color.y * clear_color.w,
        clear_color.z * clear_color.w,
        clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(gui->window);
}

static void gui__begin_root_window(gui_State *gui)
{
    (void)gui;

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
    (void)gui;

    ImGui::End();
}

b32 gui__should_close(gui_State *gui)
{
    b32 result = glfwWindowShouldClose(gui->window);

    return result;
}

static void gui__main_window(gui_State *gui, app_State *app)
{
    gui__begin_root_window(gui); {
        {
            ScopedLock lock = scoped_lock(app->error_list.mutex);

            // TODO: make it so that stacked popups work
            for (usize i = 0; i < array_count(&app->error_list); ++i) {
                app_Error error = array_at(&app->error_list, i);
                gui_popup_error(gui, error);
            }

            array_clear(&app->error_list);
        }

        gui_popup_display_active_popups(gui, app);

        if (memmi_process_is_null(app->open_process_handle)) {
            ImGui::TextUnformatted("Open process: none");
        } else {
            memmi_PID open_pid = app->open_process_handle.pid;
            char *open_pid_text = str_format_cstr(gui->frame_arena, "Open process: %" PRId64, open_pid);
            ImGui::TextUnformatted(open_pid_text);
        }

        gui_open_and_close_process_buttons(gui, app);
        ImGui::SameLine();
        gui_scan_progress_bar(gui, app);


        // NOTE: widgets wrapped in functions should never themselves call
        // BeginChild, we should only do that from top level like this
        {
            usize result_count = 0;

            if (!atomic_load_usize(&app->scan_in_progress)) {
                result_count = app->result_list.total_match_count;
            }

            char *result_count_label = str_format_cstr(
                gui->frame_arena,
                "Results: %zu",
                result_count);

            ImGui::TextUnformatted(result_count_label);

            ImGui::BeginChild("##child_result_table", RESULT_TABLE_SIZE_FRACTION, 0, 0); {
                gui_scan_result_table(gui, app);
            }; ImGui::EndChild();
        }

        ImGui::SameLine();

        s32 scan_child_flags = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders;

        ImGui::BeginChild("##child_scan", {-FLT_MIN, RESULT_TABLE_HEIGHT_FRACTION}, scan_child_flags, 0); {
            gui_scan_parameters(gui, app);
        } ImGui::EndChild();

        ImGui::BeginChild("##child_address_table", {-FLT_MIN, -FLT_MIN}, scan_child_flags, 0); {
            gui_address_table(gui, app);
        } ImGui::EndChild();
    } gui__end_window(gui);
}

void gui__update_and_render(gui_State *gui, app_State *app)
{
    arena_reset(gui->frame_arena);

    glfwPollEvents();

    if (glfwGetWindowAttrib(gui->window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
    } else {
        gui__begin_frame(gui);

        static bool show_demo_window = false;

        if (ImGui::IsKeyPressed(ImGuiKey_G)) {
            show_demo_window = !show_demo_window;
        }

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        } else {
            gui__main_window(gui, app);
        }

        gui__end_frame(gui);
    }
}

void gui_run(gui_State *gui, app_State *app)
{
    while (!gui__should_close(gui)) {
        app_update_early(app);
        gui__update_and_render(gui, app);
        app_update_late(app);
    }
}
