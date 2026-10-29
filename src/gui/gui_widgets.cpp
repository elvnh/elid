#include "gui_widgets.h"

#define GUI_MAX_DISPLAYED_RESULT_COUNT THOUSAND(50)
#define GUI_ADDRESS_AT_STATIC_OFFSET_COLOR {0, 1, 0, 1}

void gui_open_and_close_process_buttons(gui_State *gui, app_State *app)
{
    // TODO: disable if we're scanning
    if (ImGui::Button("Open process")) {
        app_action_get_running_processes(app);

        gui_popup_running_processes(gui);
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!app_has_opened_process(app)); {
        if (ImGui::Button("Close process")) {
            app_action_close_opened_process(app);
        }
    } ImGui::EndDisabled();
}

static void gui__begin_watch(gui_State *gui, app_State *app, uintptr_t address, app_WatchCondition cond, usize size)
{
    app_action_begin_watch(app, address, cond, (u32)size);
    gui_popup_address_watch(gui, app->watch);
}

static void gui__address_watch_context_menu_items(gui_State *gui, app_State *app, uintptr_t address, usize size)
{
    if (ImGui::Selectable("Find out what writes to this address")) {
        gui__begin_watch(gui, app, address, APP_WATCH_COND_WRITE, size);
    }

    if (ImGui::Selectable("Find out what accesses this address")) {
        gui__begin_watch(gui, app, address, APP_WATCH_COND_READ_WRITE, size);
    }
}

void gui_scan_result_table(gui_State *gui, app_State *app)
{
    s32 table_flags = ImGuiTableFlags_Resizable
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingStretchSame;

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

        if (!atomic_load_s32(&app->scan_in_progress)
            && (app->result_list.total_match_count > 0)
            && (app->result_list.total_match_count <= GUI_MAX_DISPLAYED_RESULT_COUNT)) {
            scan_ResultView result_view = scan_result_make_view(gui->frame_arena, app->result_list);

            ImGuiListClipper clipper = {};
            clipper.Begin(usize_to_s32(result_view.count));

            while (clipper.Step()) {
                for (s32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    // TODO: periodically update visible entries in result by looking up current value

                    scan_MatchDetails match = result_view.data[row];

                    // TODO: animate when a value changes
                    app_action_update_result_table_entry(app, match);

                    ImGui::TableNextRow();

                    b32 is_row_selected = gui->result_table.selected_row_index == row;
                    b32 match_is_at_static_offset = match.containing_object.ok;

                    // These values are loaded atomically as other threads may update them.
                    scan_Value curr_value = scan_val_atomic_load(&match.match->value);
                    scan_Value prev_value = scan_val_atomic_load(&match.history->previous_value);

                    char *addr_str = gui_cstr_from_match_address(
                        gui->frame_arena, match, app->result_list.process_objects);
                    char *curr_value_str = gui_cstr_from_match_value(
                        gui->frame_arena, curr_value, app->result_list.value_type);
                    char *prev_value_str = gui_cstr_from_match_value(
                        gui->frame_arena, prev_value, app->result_list.value_type);

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
                            if (ImGui::Selectable(curr_value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // Previous value
                        ImGui::TableSetColumnIndex(2); {
                            if (ImGui::Selectable(prev_value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // First value
                        ImGui::TableSetColumnIndex(3); {
                            char *first_value_str = gui_cstr_from_match_value(
                                gui->frame_arena, match.history->first_value, app->result_list.value_type);

                            if (ImGui::Selectable(first_value_str, is_row_selected, selectable_flags)) {
                                gui->result_table.selected_row_index = row;
                            }
                        }

                        // Right click on row context menu.
                        if (ImGui::BeginPopupContextItem()) {
                            uintptr_t address = scan_address_from_match_details(match);
                            usize value_type_size = scan_val_type_get_size(app->result_list.value_type);

                            if (ImGui::Selectable("Add to table")) {
                                app_AddressTableEntry entry = {};
                                entry.address = address;
                                entry.value_type = app->result_list.value_type;
                                entry.current_value = match.match->value;
                                entry.last_read_successful = true;

                                app_addr_table_push(&app->address_table, entry);
                            }

                            if (ImGui::Selectable("Edit this value...")) {

                                gui_popup_edit_value(
                                    gui, app->result_list.value_type, address, SOME(curr_value));
                            }

                            if (ImGui::Selectable("Copy this address")) {
                                char *address_str = str_format_cstr(gui->frame_arena, "%" PRIxPTR, address);
                                ImGui::SetClipboardText(address_str);
                            }

                            gui__address_watch_context_menu_items(gui, app, address, value_type_size);

                            ImGui::EndPopup();
                        }
                    } ImGui::PopID();
                }
            }
        }
    } ImGui::EndTable();
}

static void gui__address_table_context_menu(app_State *app, gui_State *gui, app_AddressTableEntry *entry)
{
    if (ImGui::BeginPopupContextItem()) {
        // This is done because the caller has pushed a style that causes
        // selectables to not be highlighted.
        // TODO: Handle this in a better way.
        gui_pop_no_selectable_highlight();

        if (ImGui::Selectable("Edit")) {
            gui_popup_edit_address_table_entry(gui, entry);
        }

        if (ImGui::Selectable("Edit this value...")) {
            scan_Value_Maybe value_opt = app_addr_table_try_get_entry_value(entry);

            gui_popup_edit_value(gui, entry->value_type, entry->address, value_opt);
        }

        if (ImGui::Selectable("Delete")) {
            app_action_remove_address_table_entry(app, entry);
        }

        gui__address_watch_context_menu_items(
            gui, app, entry->address, scan_val_type_get_size(entry->value_type));

        gui_push_no_selectable_highlight();

        ImGui::EndPopup();
    }
}

void gui_address_table(gui_State *gui, app_State *app)
{
    s32 table_flags = ImGuiTableFlags_Resizable
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingStretchSame;

    const char *table_columns[] = {"Address", "Type", "Value", "Decription"};

    s32 column_count = array_capacity(table_columns);
    s32 selectable_flags = ImGuiSelectableFlags_AllowDoubleClick;
    b32 double_click = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    if (ImGui::Button("Add address")) {
        gui_popup_edit_address_table_entry(gui, 0);
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

            // Turn off highlight on hover and click.
            gui_push_no_selectable_highlight();

            // Address
            ImGui::TableSetColumnIndex(0); {
                char *addr_str = gui_cstr_from_address(gui->frame_arena, entry->address);
                char *addr_id = str_format_cstr(gui->frame_arena, "%s##addr-%p", addr_str, (void *)entry);

                if (ImGui::Selectable(addr_id, false, selectable_flags) && double_click) {
                    printf("address\n");
                }

                // TODO: Make it so that we don't need to call this after every selectable
                gui__address_table_context_menu(app, gui, entry);
            }

            // Type
            ImGui::TableSetColumnIndex(1); {
                char *type_selectable_str = gui_cstr_from_value_type(gui->frame_arena, entry->value_type);
                char *type_selectable_id = str_format_cstr(
                    gui->frame_arena, "%s##type-%p", type_selectable_str, (void *)entry);

                if (ImGui::Selectable(type_selectable_id, false, selectable_flags) && double_click) {
                    printf("type\n");
                }

                gui__address_table_context_menu(app, gui, entry);
            }

            // Value
            ImGui::TableSetColumnIndex(2); {
                char *value_selectable_str = 0;

                scan_Value_Maybe value_opt = app_addr_table_try_get_entry_value(entry);

                if (value_opt.ok) {
                    value_selectable_str = gui_cstr_from_match_value(
                        gui->frame_arena, value_opt.value, entry->value_type);
                } else {
                    value_selectable_str = str_clone_cstr(gui->frame_arena, "???");
                }

                char *value_selectable_id = str_format_cstr(
                    gui->frame_arena, "%s##value-%p", value_selectable_str, (void *)entry);

                if (ImGui::Selectable(value_selectable_id, false, selectable_flags) && double_click) {
                    printf("value\n");
                }

                gui__address_table_context_menu(app, gui, entry);
            }

            // Description
            ImGui::TableSetColumnIndex(3); {
                char *description_selectable_str = str_format_cstr(gui->frame_arena, "<description>");
                char *description_selectable_id = str_format_cstr(
                    gui->frame_arena, "%s##desc-%p", description_selectable_str, (void *)entry);

                if (ImGui::Selectable(description_selectable_id, false, selectable_flags) && double_click) {
                    printf("description\n");
                }

                gui__address_table_context_menu(app, gui, entry);
            }

            gui_pop_no_selectable_highlight();
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
            app_ScannerState scanner_state = atomic_load_s32(&app->scanner_state);
            b32 scanning_disabled = !parsed_scan_input.ok
                || memmi_process_is_null(app->open_process_handle)
                || atomic_load_s32(&app->scan_in_progress);

            ImGui::BeginDisabled(scanning_disabled); {
                if (scanner_state == APP_SCANNER_STATE_FIRST_SCAN_READY) {
                    if (ImGui::Button("First scan")) {
                        app_action_run_first_scan(app, scan_settings, parsed_scan_input.value);
                    }
                } else {
                    ASSERT(scanner_state == APP_SCANNER_STATE_NEXT_SCAN_READY);

                    if (ImGui::Button("New scan")) {
                        app_action_reset_scan_results(app);
                    }
                }

                ImGui::SameLine();

                b32 next_scan_disabled = scanner_state != APP_SCANNER_STATE_NEXT_SCAN_READY
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
            gui, "Value type: ", selected_type, combo_box_width);
    } ImGui::EndChild();

    // Scan settings (fast scan, pause while scanning, etc.)
    ImGui::BeginChild("##child_scan_settings", {-FLT_MIN, 0}, child_flags, 0); {
        ImGui::Checkbox("Fast scan", &gui->scan_parameters.fast_scan);
        ImGui::Checkbox("Pause while scanning", &gui->scan_parameters.pause_while_scanning);
    } ImGui::EndChild();
}

