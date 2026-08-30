#include "gui_widgets.h"

#define GUI_OPEN_PROC_POPUP_NAME "Open process"

void gui_open_process_button(gui_State *gui, app_State *app)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_action_get_running_processes(app);

        gui->process_list_popup.selected_index = -1;
        ImGui::OpenPopup(GUI_OPEN_PROC_POPUP_NAME);
    }
}

void gui_open_process_popup(gui_State *gui, app_State *app)
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

            should_close = app_action_open_process(app, clicked_process.pid);
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

void gui_scan_result_table(gui_State *gui, app_State *app)
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

void gui_scan_parameters(gui_State *gui, app_State *app)
{
    s32 child_flags = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders;

    // Scan parameters (value, type, etc.)
    ImGui::BeginChild("##child_scan_params", {-FLT_MIN, 0}, child_flags, 0); {
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

        u32 scan_value_input_base = gui->scan_parameters.hex ? 16 : 10;
        scan_Value_Maybe parsed_scan_value = gui_try_parse_search_value(
            gui->scan_parameters.scan_value_text,
            gui->scan_parameters.selected_value_type,
            scan_value_input_base);

        // Scan buttons
        {
            ImGui::BeginDisabled(!parsed_scan_value.ok); {
                if (ImGui::Button("New/first scan")) {
                    TODO("scan");
                }

                ImGui::SameLine();

                if (ImGui::Button("Next scan")) {
                    TODO("scan");
                }
            } ImGui::EndDisabled();
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

            scan_ModeList available_modes = scan_mode_get_list();
            const char *scan_mode_preview = scan_mode_to_cstr(gui->scan_parameters.selected_scan_mode);

            // TODO: if the selected mode is no longer available, default to the first one in list

            ImGui::SetNextItemWidth(combo_box_width);

            if (ImGui::BeginCombo("##scan_mode", scan_mode_preview, combo_flags)) {
                for (size_t i = 0; i < available_modes.count; ++i) {
                    scan_Mode mode = available_modes.data[i];

                    b32 is_mode_selected = gui->scan_parameters.selected_scan_mode == mode;

                    const char *scan_mode_name = scan_mode_to_cstr(mode);

                    if (ImGui::Selectable(scan_mode_name, is_mode_selected)) {
                        gui->scan_parameters.selected_scan_mode = mode;
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

            scan_ValueTypeList available_types = scan_val_type_get_list();
            const char *value_type_preview = scan_val_type_to_cstr(gui->scan_parameters.selected_value_type);

            ImGui::SetNextItemWidth(combo_box_width);

            if (ImGui::BeginCombo("##value_type", value_type_preview, combo_flags)) {
                for (size_t i = 0; i < available_types.count; ++i) {
                    scan_ValueType type = available_types.data[i];
                    b32 is_type_selected = gui->scan_parameters.selected_value_type == type;

                    const char *type_spelling = scan_val_type_to_cstr(type);

                    if (ImGui::Selectable(type_spelling, is_type_selected)) {
                        gui->scan_parameters.selected_value_type = type;
                    }
                }

                ImGui::EndCombo();
            }
        }

    } ImGui::EndChild();

    // Scan settings (fast scan, pause while scanning, etc.)
    ImGui::BeginChild("##child_scan_settings", {-FLT_MIN, 0}, child_flags, 0); {
        ImGui::Checkbox("Fast scan", &gui->scan_parameters.fast_scan);
        ImGui::Checkbox("Pause while scanning", &gui->scan_parameters.pause_while_scanning);
    } ImGui::EndChild();
}

