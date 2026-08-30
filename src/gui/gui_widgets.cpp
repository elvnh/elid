#include "gui_widgets.h"

#define GUI_MAX_DISPLAYED_RESULT_COUNT THOUSAND(50)

void gui_open_process_button(gui_State *gui, app_State *app)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_action_get_running_processes(app);

        gui->process_list_popup.selected_index = -1;
        gui_popup_open(app->frame_arena, GUI_POPUP_OPEN_PROCESS);
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
    s32 column_count = array_capacity(table_columns);

    // TODO: turn off highlight on hover

    if (ImGui::BeginTable("#scan_result_table", column_count, table_flags, {})) {
        ImGui::TableSetupScrollFreeze(0, 1);

        for (s32 col = 0; col < column_count; ++col) {
            ImGui::TableSetupColumn(table_columns[col]);
        }

        ImGui::TableHeadersRow();

        if (app->result_list.total_match_count <= GUI_MAX_DISPLAYED_RESULT_COUNT) {
            s32 match_count = usize_to_s32(app->result_list.total_match_count);

            ImGuiListClipper clipper = {};
            clipper.Begin(match_count);

            while (clipper.Step()) {
                for (s32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    // TODO: optimize lookup of results
                    // TODO: green text if located inside executable/so file
                    // TODO: show previous value
                    scan_MatchDetails match = scan_result_at(&app->result_list, (usize)row);

                    ImGui::TableNextRow();

                    b32 is_row_selected = gui->result_table.selected_row_index == row;

                    ImGui::TableSetColumnIndex(0); {
                        char *addr_str = str_format_cstr(app->frame_arena, "%" PRIxPTR, match.address);

                        if (ImGui::Selectable(addr_str, is_row_selected, selectable_flags)) {
                            gui->result_table.selected_row_index = row;
                        }
                    }

                    ImGui::TableSetColumnIndex(1); {
                        char *value_str = str_format_cstr(app->frame_arena, "%u", match.value.int32);

                        if (ImGui::Selectable(value_str, is_row_selected, selectable_flags)) {
                            gui->result_table.selected_row_index = row;
                        }
                    }

                    ImGui::TableSetColumnIndex(2); {
                        if (ImGui::Selectable("", is_row_selected, selectable_flags)) {
                            gui->result_table.selected_row_index = row;
                        }
                    }
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

            b32 input_not_required = scan_mode_get_arity(gui->scan_parameters.selected_scan_mode) == 0;

            ImGui::BeginDisabled(input_not_required); {
                ImGui::InputText(
                    "##scan_value",
                    gui->scan_parameters.scan_value_text,
                    array_capacity(gui->scan_parameters.scan_value_text)
                );
            } ImGui::EndDisabled();
        }

        u32 scan_value_input_base = gui->scan_parameters.hex ? 16 : 10;

        scan_Comparison_Maybe parsed_scan_input = gui_try_parse_scan_input(
            gui->scan_parameters.scan_value_text,
            gui->scan_parameters.selected_value_type,
            scan_value_input_base,
            gui->scan_parameters.selected_scan_mode
        );

        usize scan_stride = 4;

        // Scan buttons
        {
            b32 scanning_disabled = !parsed_scan_input.ok
                || memmi_process_is_null(app->open_process_handle);

            ImGui::BeginDisabled(scanning_disabled); {
                if (app->scanner_state == APP_SCANNER_STATE_FIRST_SCAN_READY) {
                    if (ImGui::Button("First scan")) {
                        // TODO: show error if we don't have permissions to scan
                        app_action_run_first_scan(app, parsed_scan_input.value, scan_stride);
                    }
                } else if (ALWAYS(app->scanner_state == APP_SCANNER_STATE_NEXT_SCAN_READY)) {
                    if (ImGui::Button("New scan")) {
                        app_action_reset_scan_results(app);
                    }
                }

                ImGui::SameLine();

                b32 next_scan_disabled = app->scanner_state != APP_SCANNER_STATE_NEXT_SCAN_READY
                    || (app->result_list.total_match_count == 0);

                ImGui::BeginDisabled(next_scan_disabled); {
                    if (ImGui::Button("Next scan")) {
                        app_action_run_next_scan(app, parsed_scan_input.value);
                    }
                } ImGui::EndDisabled();

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
                for (usize i = 0; i < available_modes.count; ++i) {
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
                for (usize i = 0; i < available_types.count; ++i) {
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

