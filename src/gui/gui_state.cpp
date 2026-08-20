#include "gui_state.h"

#define RESULT_TABLE_SIZE_FRACTION {ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y * 0.8f}

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

b32 gui_should_close(gui_State *gui)
{
    b32 result = glfwWindowShouldClose(gui->window);

    return result;
}

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

#define GUI_OPEN_PROC_POPUP_NAME "Open process"

static void gui__do_open_process_button(gui_State *gui, app_State *app, app_CommandList *commands)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_command_push(app->frame_arena, commands, APP_COMMAND_GET_RUNNING_PROCS);

        gui->process_list_popup.selected_index = -1;
        ImGui::OpenPopup(GUI_OPEN_PROC_POPUP_NAME);
    }
}

static void gui__do_open_process_popup(gui_State *gui, app_State *app, app_CommandList *commands)
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

static void gui__do_scan_result_table(gui_State *gui, app_State *app, app_CommandList *commands)
{
    s32 table_flags = ImGuiTableFlags_Resizable
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollY;
    s32 selectable_flags = ImGuiSelectableFlags_SpanAllColumns;

    const char *table_columns[] = {"Address", "Value", "Previous"};
    s32 column_count = ARRAY_COUNT(table_columns);

    if (ImGui::BeginTable("#scan_result_table", column_count, table_flags, {})) {
        ImGui::TableSetupScrollFreeze(0, 1);

        for (s32 col = 0; col < column_count; ++col) {
            ImGui::TableSetupColumn(table_columns[col]);
        }

        ImGui::TableHeadersRow();

        s32 row_count = 10;

        for (s32 row = 0; row < row_count; ++row) {
            ImGui::TableNextRow();

            b32 is_row_selected = gui->result_table.selected_row_index == row;

            // TODO: use address as id
            ImGui::TableSetColumnIndex(0); {
                char *addr_str = str_format_cstr(app->frame_arena, "%d", row);
                if (ImGui::Selectable(addr_str, is_row_selected, selectable_flags)) {
                    gui->result_table.selected_row_index = row;
                }
            }

            ImGui::TableSetColumnIndex(1); {
                char *value_str = str_format_cstr(app->frame_arena, "value %d", row);
                if (ImGui::Selectable(value_str, is_row_selected, selectable_flags)) {
                    gui->result_table.selected_row_index = row;
                }
            }

            ImGui::TableSetColumnIndex(2); {
                char *value_str = str_format_cstr(app->frame_arena, "prev value %d", row);
                if (ImGui::Selectable(value_str, is_row_selected, selectable_flags)) {
                    gui->result_table.selected_row_index = row;
                }
            }
        }
    } ImGui::EndTable();
}

static void gui__do_scan_widgets(gui_State *gui, app_State *app, app_CommandList *commands, s32 child_flags)
{
    // Scan parameters (value, type, etc.)
    ImGui::BeginChild("##child_scan_params", {-FLT_MIN, 0}, child_flags | ImGuiChildFlags_Borders, 0); {
        // Scan input
        {
            ImGui::TextUnformatted("Scan:");
            ImGui::SameLine();

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##scan_value",
                gui->scan_parameters.scan_value_text,
                ARRAY_COUNT(gui->scan_parameters.scan_value_text)
            );
        }

        // Scan buttons
        {
            if (ImGui::Button("New/first scan")) {
                TODO("scan");
            }

            ImGui::SameLine();

            if (ImGui::Button("Next scan")) {
                TODO("scan");
            }
        }

        // Hex?
        {
            ImGui::Checkbox("Hex", &gui->scan_parameters.hex);
        }

        // TODO: don't hardcode this, find how much space it takes by checking max length
        // of all scan mode/value type strings
        f32 combo_box_width = 200;

        // Scan mode
        {
            ImGui::TextUnformatted("Scan mode: ");
            ImGui::SameLine();

            s32 combo_flags = ImGuiComboFlags_HeightLargest;

            const char *scan_mode_names[] = {"Foo", "Bar"};
            const char *scan_mode_preview = scan_mode_names[gui->scan_parameters.selected_scan_mode_index];

            ImGui::SetNextItemWidth(combo_box_width);

            if (ImGui::BeginCombo("##scan_mode", scan_mode_preview, combo_flags)) {
                for (s32 i = 0; i < (s32)ARRAY_COUNT(scan_mode_names); ++i) {
                    b32 is_mode_selected = gui->scan_parameters.selected_scan_mode_index == i;

                    if (ImGui::Selectable(scan_mode_names[i], is_mode_selected)) {
                        gui->scan_parameters.selected_scan_mode_index = i;
                    }
                }

                ImGui::EndCombo();
            }
        }

        // Value type
        {
            ImGui::TextUnformatted("Value type:");
            ImGui::SameLine();

            s32 combo_flags = ImGuiComboFlags_HeightLargest;

            const char *value_type_names[] = {"Flup", "Drup"};
            const char *value_type_preview = value_type_names[gui->scan_parameters.selected_value_type_index];

            ImGui::SetNextItemWidth(combo_box_width);

            if (ImGui::BeginCombo("##value_type", value_type_preview, combo_flags)) {
                for (s32 i = 0; i < (s32)ARRAY_COUNT(value_type_names); ++i) {
                    b32 is_mode_selected = gui->scan_parameters.selected_value_type_index == i;

                    if (ImGui::Selectable(value_type_names[i], is_mode_selected)) {
                        gui->scan_parameters.selected_value_type_index = i;
                    }
                }

                ImGui::EndCombo();
            }
        }

    } ImGui::EndChild();

    // Scan settings (fast scan, pause while scanning, etc.)
    ImGui::BeginChild("##child_scan_settings", {-FLT_MIN, 0}, child_flags | ImGuiChildFlags_Borders, 0); {
        ImGui::Checkbox("Fast scan", &gui->scan_parameters.fast_scan);
        ImGui::Checkbox("Pause while scanning", &gui->scan_parameters.pause_while_scanning);
    } ImGui::EndChild();
}

app_CommandList gui_update_and_render(gui_State *gui, app_State *app)
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
                    ImGui::TextUnformatted("Open process: none");
                } else {
                    memmi_PID open_pid = app->open_process_handle.pid;
                    char *open_pid_text = str_format_cstr(app->frame_arena, "Open process: %" PRId64, open_pid.value);
                    ImGui::TextUnformatted(open_pid_text);
                }

                gui__do_open_process_button(gui, app, &result);
                gui__do_open_process_popup(gui, app, &result);

                ImGui::TextUnformatted("Results: 0");

                // NOTE: widgets wrapped in functions should never themselves call
                // BeginChild, we should only do that from top level like this
                ImGui::BeginChild("##child_result_table", RESULT_TABLE_SIZE_FRACTION, 0, 0); {
                    gui__do_scan_result_table(gui, app, &result);
                }; ImGui::EndChild();

                ImGui::SameLine();

                s32 scan_child_flags = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;

                ImGui::BeginChild("##child_scan", {-FLT_MIN, -FLT_MIN}, scan_child_flags | ImGuiChildFlags_Borders, 0); {
                    gui__do_scan_widgets(gui, app, &result, scan_child_flags);
                } ImGui::EndChild();

            } gui__end_window(gui);
        }



        gui__end_frame(gui);
    }

    return result;
}
