#include "gui_widgets.h"

#define GUI_MAX_DISPLAYED_RESULT_COUNT THOUSAND(50)
#define GUI_ADDRESS_AT_STATIC_OFFSET_COLOR {0, 1, 0, 1}

void gui_open_process_button(gui_State *gui, app_State *app)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_action_get_running_processes(app);

        gui->process_list_popup.selected_index = -1;
        gui_popup_open(gui, GUI_POPUP_OPEN_PROCESS);
    }
}

void gui_scan_result_table(gui_State *gui, app_State *app)
{
    s32 table_flags = ImGuiTableFlags_Resizable
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollY;

    s32 selectable_flags = ImGuiSelectableFlags_SpanAllColumns;

    const char *table_columns[] = {"Address", "Value", "Previous", "First"};
    s32 column_count = array_capacity(table_columns);

    // TODO: turn off highlight on hover

    if (ImGui::BeginTable("#scan_result_table", column_count, table_flags, {})) {
        ImGui::TableSetupScrollFreeze(0, 1);

        for (s32 col = 0; col < column_count; ++col) {
            ImGui::TableSetupColumn(table_columns[col]);
        }

        ImGui::TableHeadersRow();

        if ((app->result_list.total_match_count > 0)
            && (app->result_list.total_match_count <= GUI_MAX_DISPLAYED_RESULT_COUNT)) {
            scan_ResultView result_view = scan_result_make_view(app->frame_arena, app->result_list);

            ImGuiListClipper clipper = {};
            clipper.Begin(usize_to_s32(result_view.count));

            while (clipper.Step()) {
                for (s32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    // TODO: optimize lookup of results
                    // TODO: periodically update visible entries in result by looking up current value

                    scan_MatchDetails match = result_view.data[row];

                    ImGui::TableNextRow();

                    b32 is_row_selected = gui->result_table.selected_row_index == row;
                    b32 match_is_at_static_offset = match.containing_object.ok;

                    char *addr_str = gui_cstr_from_match_address(
                        app->frame_arena, match, app->result_list.process_objects);
                    char *value_str = gui_cstr_from_match_value(
                        app->frame_arena, match.match->value, app->result_list.value_type);

                    // TODO: is this PushID necessary?
                    ImGui::PushID(row); {
                        // Address
                        ImGui::TableSetColumnIndex(0); {
                            // Display addresses inside process image as green.
                            if (match_is_at_static_offset) {
                                ImGui::PushStyleColor(ImGuiCol_Text, GUI_ADDRESS_AT_STATIC_OFFSET_COLOR);
                            }

                            if (ImGui::Selectable(addr_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }

                            if (match_is_at_static_offset) {
                                ImGui::PopStyleColor();
                            }
                        }

                        // Currrent value
                        ImGui::TableSetColumnIndex(1); {
                            if (ImGui::Selectable(value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // Previous value
                        ImGui::TableSetColumnIndex(2); {
                            char *prev_value_str = gui_cstr_from_match_value(
                                app->frame_arena, match.history->previous_value, app->result_list.value_type);

                            if (ImGui::Selectable(prev_value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // First value
                        ImGui::TableSetColumnIndex(3); {
                            char *first_value_str = gui_cstr_from_match_value(
                                app->frame_arena, match.history->first_value, app->result_list.value_type);

                            if (ImGui::Selectable(first_value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // Right click on row context menu.
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::Selectable("Add to table")) {
                                app_AddressTableEntry entry = {};
                                entry.address = scan_address_from_match_details(match);
                                entry.value_type = app->result_list.value_type;
                                entry.current_value = match.match->value;
                                entry.last_read_successful = true;

                                app_addr_table_push(&app->address_table, entry);
                            }

                            if (ImGui::Selectable("Edit this value...")) {
                                uintptr_t address = scan_address_from_match_details(match);

                                gui->edit_value_popup.edited_address = address;

                                strncpy(gui->edit_value_popup.value_textbox,
                                    value_str,
                                    array_capacity(gui->edit_value_popup.value_textbox));

                                gui->edit_value_popup.value_type = app->result_list.value_type;

                                gui_popup_open(gui, GUI_POPUP_EDIT_VALUE);
                            }

                            if (ImGui::Selectable("Copy this address")) {
                                uintptr_t address = scan_address_from_match_details(match);

                                char *address_str = str_format_cstr(app->frame_arena, "%" PRIxPTR, address);
                                ImGui::SetClipboardText(address_str);
                            }

                            ImGui::EndPopup();
                        }
                    } ImGui::PopID();
                }
            }
        }
    } ImGui::EndTable();
}

void gui_address_table(gui_State *gui, app_State *app)
{
    s32 table_flags = ImGuiTableFlags_Resizable
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollY;

    const char *table_columns[] = {"Address", "Type", "Value"};

    s32 column_count = array_capacity(table_columns);

    if (ImGui::Button("Add address")) {
        gui_popup_open(gui, GUI_POPUP_ADD_ADDRESS);
    }

    if (ImGui::BeginTable("##address_table", column_count, table_flags, {})) {
        ImGui::TableSetupScrollFreeze(0, 1);

        for (s32 col = 0; col < column_count; ++col) {
            ImGui::TableSetupColumn(table_columns[col]);
        }

        ImGui::TableHeadersRow();

        app_AddressTable *tbl = &app->address_table;

        for (app_AddressTableEntry *entry = list_head(tbl); entry; entry = list_next(entry)) {
            ImGui::TableNextRow();

            app_action_update_address_table_entry_value(app, entry);

            // Address
            ImGui::TableSetColumnIndex(0); {
                char *addr_str = gui_cstr_from_address(app->frame_arena, entry->address);
                char *addr_id = str_format_cstr(app->frame_arena, "%s##%p", addr_str, (void *)entry);

                if (ImGui::Selectable(addr_id, false, 0)) {

                }
            }

            // Type
            ImGui::TableSetColumnIndex(1); {
                char *type_selectable_str = gui_cstr_from_value_type(app->frame_arena, entry->value_type);
                char *type_selectable_id = str_format_cstr(app->frame_arena, "%s##%p", type_selectable_str, (void *)entry);

                if (ImGui::Selectable(type_selectable_id, false, 0)) {

                }
            }

            // Value
            ImGui::TableSetColumnIndex(2); {
                char *value_selectable_str = 0;

                if (entry->last_read_successful) {
                    value_selectable_str = gui_cstr_from_match_value(app->frame_arena, entry->current_value, entry->value_type);
                } else {
                    value_selectable_str = str_clone_cstr(app->frame_arena, "???");
                }

                char *value_selectable_id = str_format_cstr(app->frame_arena, "%s##%p", value_selectable_str, (void *)entry);

                if (ImGui::Selectable(value_selectable_id, false, 0)) {

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

        scan_Settings scan_settings = gui_get_scan_settings(gui);

        // Scan buttons
        {
            b32 scanning_disabled = !parsed_scan_input.ok
                || memmi_process_is_null(app->open_process_handle);

            ImGui::BeginDisabled(scanning_disabled); {
                if (app->scanner_state == APP_SCANNER_STATE_FIRST_SCAN_READY) {
                    if (ImGui::Button("First scan")) {
                        app_action_run_first_scan(app, scan_settings, parsed_scan_input.value);
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
                        app_action_run_next_scan(app, scan_settings, parsed_scan_input.value);
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
        scan_ValueType selected_type = gui->scan_parameters.selected_value_type;

        gui->scan_parameters.selected_value_type = gui_value_type_dropdown(
            app, "Value type: ", selected_type, combo_box_width);
    } ImGui::EndChild();

    // Scan settings (fast scan, pause while scanning, etc.)
    ImGui::BeginChild("##child_scan_settings", {-FLT_MIN, 0}, child_flags, 0); {
        ImGui::Checkbox("Fast scan", &gui->scan_parameters.fast_scan);
        ImGui::Checkbox("Pause while scanning", &gui->scan_parameters.pause_while_scanning);
    } ImGui::EndChild();
}

