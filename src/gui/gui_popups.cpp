#include "gui_popups.h"

char *gui__popup_get_name(Arena *arena, gui_Popup popup, const char *title)
{
    if (!title) {
        title = "";
    }

    // Only the popup enum will contribute to the ID, meaning we can provide different titles
    // when opening vs when defining the popup window.
    const char *id_str = str_format_cstr(arena, "%d", (s32)popup);
    ImGuiID id = ImGui::GetID(id_str);

    char *result = str_format_cstr(arena, "%s###%u", title, id);

    return result;
}

void gui__popup_open(gui_State *gui, gui_Popup popup)
{
    ASSERT(popup != GUI_POPUP_COUNT);

    gui->popup_states[popup] = true;
}

void gui_popup_running_processes(gui_State *gui)
{
    gui->process_list_popup.selected_index = -1;
    zero_mem(gui->process_list_popup.search_textbox, sizeof(gui->process_list_popup.search_textbox));

    gui__popup_open(gui, GUI_POPUP_OPEN_PROCESS);
}

void gui_popup_edit_value(gui_State *gui, scan_ValueType type, uintptr_t address, scan_Value_Maybe curr_value)
{
    gui->edit_value_popup.edited_address = address;

    const char *value_str = 0;

    if (curr_value.ok) {
        value_str = gui_cstr_from_match_value(
            gui->frame_arena, curr_value.value, type);
    } else {
        value_str = "";
    }

    strncpy(gui->edit_value_popup.value_textbox,
        value_str,
        array_capacity(gui->edit_value_popup.value_textbox) - 1);

    gui__popup_open(gui, GUI_POPUP_EDIT_VALUE);
}

void gui_popup_edit_address_table_entry(gui_State *gui, app_AddressTableEntry *optional_entry)
{
    pool_reset(gui->edit_address_table_entry_popup.entry_pool);

    if (optional_entry) {
        // Edit an existing entry.
        gui->edit_address_table_entry_popup.selected_value_type = optional_entry->value_type;
        gui->edit_address_table_entry_popup.existing_entry = optional_entry;

        // Make a copy of the entry, to be saved later.
        gui->edit_address_table_entry_popup.entry = app_addr_table_entry_clone(
            gui->edit_address_table_entry_popup.entry_pool,
            optional_entry);

        char *address_str = str_format_cstr(gui->frame_arena, "%" PRIxPTR, optional_entry->address);
        strncpy(gui->edit_address_table_entry_popup.address_textbox,
            address_str,
            array_capacity(gui->edit_address_table_entry_popup.address_textbox) - 1);
    } else {
        // Creating a new entry. Reset popup state to zero (except for the pool).
        Pool *pool = gui->edit_address_table_entry_popup.entry_pool;
        gui->edit_address_table_entry_popup = {};
        gui->edit_address_table_entry_popup.entry_pool = pool;

        gui->edit_address_table_entry_popup.entry =
            app_addr_table_entry_alloc(gui->edit_address_table_entry_popup.entry_pool);

    }

    app_addr_table_entry_initialize_textboxes(gui->edit_address_table_entry_popup.entry);

    gui__popup_open(gui, GUI_POPUP_ADD_ADDRESS);
}

void gui_popup_address_watch(gui_State *gui, app_Watch *watch)
{
    ASSERT(watch);

    gui->address_watch_popup.watch = watch;
    gui->address_watch_popup.selected_instruction = 0;

    gui__popup_open(gui, GUI_POPUP_WATCH);
}

void gui_popup_close(gui_State *gui, gui_Popup popup)
{
    ASSERT(popup != GUI_POPUP_COUNT);

    gui->popup_states[popup] = false;
    ImGui::CloseCurrentPopup();
}

static void gui__set_initial_popup_size(f32 width_fraction, f32 height_fraction)
{
    ASSERT((width_fraction >= 0.0f) || (fabsf(width_fraction) == FLT_MIN));
    ASSERT((width_fraction <= 1.0f));
    ASSERT((height_fraction >= 0.0f) || (fabsf(height_fraction) == FLT_MIN));
    ASSERT((height_fraction <= 1.0f));

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});

    ImVec2 window_size = {
        root_window_size.x * width_fraction,
        root_window_size.y * height_fraction
    };

    if (fabsf(width_fraction) == FLT_MIN) {
        window_size.x = width_fraction;
    }

    if (fabsf(height_fraction) == FLT_MIN) {
        window_size.y = height_fraction;
    }

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Appearing);
}

static void gui__popup_open_process(gui_State *gui, app_State *app)
{
    // TODO: display PID plus name, and allow searching for pid rather than just name

    gui__set_initial_popup_size(0.75f, 0.75f);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    const char *name = gui__popup_get_name(gui->frame_arena, GUI_POPUP_OPEN_PROCESS, "Open process");

    if (ImGui::BeginPopupModal(name, 0, popup_flags)) {
        // Search textbox
        gui_set_initial_keyboard_focus_here();

        ImGui::SetNextItemWidth(-ImGui::GetFrameHeight());
        ImGuiInputTextFlags input_text_flags = {};
        ImGui::InputText("##process_list_search",
            gui->process_list_popup.search_textbox, sizeof(gui->process_list_popup.search_textbox), input_text_flags);

        // Listbox
        s32 process_count = 0;

        b32 is_ready = atomic_load_s32(&app->process_list_ready);

        if (is_ready) {
            process_count = usize_to_s32(app->running_process_list.count);
        }

        // TODO: show number of processes
        // TODO: remove selected entry if process list changes

        b32 item_is_highlighted = false;
        u32 new_filtered_process_count = 0;

        ImVec2 listbox_size = {-FLT_MIN, -ImGui::GetFrameHeight()};

        if (ImGui::BeginListBox("##process_list", listbox_size)) {
            for (s32 i = 0; i < process_count; ++i) {
                memmi_ProcessInfo proc = app->running_process_list.data[i];
                ASSERT(proc.name.data);
                ASSERT(proc.name.count > 0);

                String proc_name = {(u8 *)proc.name.data, proc.name.count};
                char *proc_name_cstr = str_to_cstr(gui->frame_arena, proc_name);

                String textbox_contents = str_from_cstr(gui->process_list_popup.search_textbox);
                StringSearch search_result = str_search(proc_name, textbox_contents, STR_SEARCH_CASE_INSENSITIVE);

                if (search_result.match) {
                    ++new_filtered_process_count;

                    const char *proc_id = str_format_cstr(gui->frame_arena, "process_%d", s64_to_s32(proc.pid));

                    ImGui::PushID(proc_id); {
                        b32 is_selected = gui->process_list_popup.selected_index == i;

                        if (ImGui::Selectable(proc_name_cstr, is_selected)) {
                            gui->process_list_popup.selected_index = i;
                        }

                        if (ImGui::IsItemHovered()) {
                            item_is_highlighted = true;
                        }
                    } ImGui::PopID();
                }
            }

            ImGui::EndListBox();
        }

        if ((new_filtered_process_count != gui->process_list_popup.previous_filtered_process_count) &&
            (gui->process_list_popup.previous_filtered_process_count != 0)) {
            // We don't want the selected item to persist when changing search text, reset it.
            gui->process_list_popup.selected_index = -1;
        }

        gui->process_list_popup.previous_filtered_process_count = new_filtered_process_count;

        // TODO: can selected index be a local variable?
        s32 selected_index = gui->process_list_popup.selected_index;
        b32 is_valid_index = (selected_index >= 0) && (selected_index < process_count);
        b32 has_double_clicked = item_is_highlighted && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

        b32 should_close = false;
        b32 should_open_process = false;

        ImGui::BeginDisabled(!is_ready || !is_valid_index); {
            if ((ImGui::Button("Open") || has_double_clicked) && is_valid_index) {
                should_open_process = true;
            }
        } ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            should_close = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Refresh")) {
            app_action_get_running_processes(app);
        }

        // Keyboard controls
        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if (is_valid_index) {
                should_open_process = true;
            }
        } else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            should_close = true;
        }

        if (should_open_process) {
            ASSERT(selected_index >= 0);
            ASSERT(selected_index < process_count);
            ASSERT(is_valid_index);

            memmi_ProcessInfo clicked_process = app->running_process_list.data[selected_index];

            should_close = app_action_open_process(app, clicked_process.pid);
        }

        if (should_close) {
            gui_popup_close(gui, GUI_POPUP_OPEN_PROCESS);
        }

        ImGui::EndPopup();
    }
}

static void gui__generic_error_popup(gui_State *gui, gui_Popup popup, const char *text)
{
    gui__set_initial_popup_size(0.75f, 0.75f);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    const char *popup_name = gui__popup_get_name(gui->frame_arena, popup, "Error");

    if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
        ImGui::TextUnformatted(text);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            gui_popup_close(gui, popup);
        }

        ImGui::EndPopup();
    }
}

static void gui__popup_edit_value(gui_State *gui, app_State *app)
{
    gui__set_initial_popup_size(-FLT_MIN, 0.75f);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    const char *title = str_format_cstr(gui->frame_arena, "Edit value at %" PRIxPTR, gui->edit_value_popup.edited_address);
    const char *popup_name = gui__popup_get_name(gui->frame_arena, GUI_POPUP_EDIT_VALUE, title);

    if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(
            "##edit_value",
            gui->edit_value_popup.value_textbox,
            array_capacity(gui->edit_value_popup.value_textbox)
        );

        ImGui::Separator();

        b32 should_close = false;

        uintptr_t address = gui->edit_value_popup.edited_address;
        scan_ValueType value_type = gui->edit_value_popup.value_type;
        u32 base = 10; // TODO: allow customizing

        scan_Value_Maybe input_value = gui_try_parse_search_value(
            gui->edit_value_popup.value_textbox, value_type, base);

        ImGui::BeginDisabled(!input_value.ok); {
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ASSERT(input_value.ok);

                app_action_edit_value_at_address(app, address, value_type, input_value.value);

                should_close = true;
            }
        } ImGui::EndDisabled();


        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            should_close = true;
        }

        if (should_close) {
            gui_popup_close(gui, GUI_POPUP_EDIT_VALUE);
        }

        ImGui::EndPopup();
    }
}

static void gui__popup_edit_address_table_entry(gui_State *gui, app_State *app)
{
    gui__set_initial_popup_size(-FLT_MIN, 0.75f);

    s32 popup_flags =
        ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_AlwaysAutoResize;

    const char *title = "Add address";
    const char *popup_name = gui__popup_get_name(gui->frame_arena, GUI_POPUP_ADD_ADDRESS, title);

    if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
        // A private copy of the entry we are editing, to be saved later.
        app_AddressTableEntry *entry = gui->edit_address_table_entry_popup.entry;

        // The entry the user has chosen to edit, null if this is an entirely new entry.
        app_AddressTableEntry *existing_entry = gui->edit_address_table_entry_popup.existing_entry;

        ASSERT(entry->as_pointer_chain.sentinel);
        ASSERT(entry->as_pointer_chain.sentinel->value_type == entry->value_type);

        ASSERT(entry->as_pointer_chain.offset == 0);
        ASSERT(entry->as_pointer_chain.sentinel->as_pointer_chain.offset == 0);

        b32 parsing_ok = false;
        b32 should_close_popup = false;
        b32 should_save_entry = false;

        // Parse input and try to fetch the value of the entry.
        {
            app_addr_table_entry_set_type(entry, gui->edit_address_table_entry_popup.selected_value_type);

            u32 address_number_base = 16;

            parsing_ok = gui_try_parse_address(
                gui->edit_address_table_entry_popup.address_textbox,
                address_number_base,
                &entry->address
            );

            if (gui->edit_address_table_entry_popup.is_pointer) {
                if (entry->kind != APP_ADDR_TABLE_ENTRY_POINTER) {
                    // We just changed from a non-pointer to a pointer entry, so we'll
                    // have to do some initialization.
                    app_addr_table_entry_initialize_textboxes(entry);
                }

                entry->kind = APP_ADDR_TABLE_ENTRY_POINTER;
            } else {
                entry->kind = APP_ADDR_TABLE_ENTRY_NORMAL;
            }

            app_action_update_address_table_entry_synchronously(app, entry);
        }

        // Widgets.
        {
            ImGui::TextUnformatted("Address:");

            ImGui::SameLine();

            ImGuiInputTextFlags input_text_flags =
                ImGuiInputTextFlags_EnterReturnsTrue;

            gui_set_initial_keyboard_focus_here();

            ImGui::BeginDisabled(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER); {
                char *address_buffer = 0;
                usize address_buffer_length = 0;

                if (entry->kind == APP_ADDR_TABLE_ENTRY_NORMAL) {
                    address_buffer = gui->edit_address_table_entry_popup.address_textbox;
                    address_buffer_length =
                        array_capacity(gui->edit_address_table_entry_popup.address_textbox);
                } else {
                    // If this is a pointer chain, try to display the final address instead of the
                    // textbox contents.
                    uintptr_t address = 0;

                    if (app_addr_table_entry_try_get_final_address(entry, &address)) {
                        address_buffer = str_format_cstr(gui->frame_arena, "%" PRIxPTR, address);
                    } else {
                        address_buffer = str_clone_cstr(gui->frame_arena, "???");
                    }

                    address_buffer_length = strlen(address_buffer);
                }

                if (ImGui::InputText(
                        "##address",
                        address_buffer,
                        address_buffer_length,
                        input_text_flags)) {
                    should_save_entry = true;
                }
            } ImGui::EndDisabled();

            ImGui::SameLine();

            // Display the current value at address if possible.
            {
                const char *current_value_str = 0;

                scan_Value current_value = {};

                if (app_addr_table_entry_try_get_final_value(entry, &current_value)) {
                    current_value_str =
                        gui_cstr_from_match_value(gui->frame_arena, current_value, entry->value_type);
                } else {
                    // TODO: create a constant for unknown value string
                    current_value_str = "???";
                }

                char *value_label_str = str_format_cstr(gui->frame_arena, " = %s", current_value_str);
                ImGui::TextUnformatted(value_label_str);
            }

            ImGui::Separator();

            f32 combo_box_width = 200;

            scan_ValueType current_value_type = gui->edit_address_table_entry_popup.selected_value_type;
            gui->edit_address_table_entry_popup.selected_value_type =
                gui_value_type_dropdown(gui, "Value type:", current_value_type, combo_box_width);

            ImGui::Checkbox("Pointer", &gui->edit_address_table_entry_popup.is_pointer);

            // Display pointer offsets.
            if (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER) {
                // TODO: fix this size
                ImVec2 table_size = {-FLT_MIN, 200};

                s32 table_flags = ImGuiTableFlags_Resizable
                    | ImGuiTableFlags_BordersV
                    | ImGuiTableFlags_Borders
                    | ImGuiTableFlags_ScrollY
                    | ImGuiTableFlags_SizingStretchSame;

                const char *table_columns[] = {"Offset", "Result"};
                s32 column_count = array_capacity(table_columns);


                ImGui::BeginChild("##pointer_chain_child", table_size); {

                    if (ImGui::BeginTable("#pointer_chain_table", column_count, table_flags, {})) {
                        ImGui::TableSetupScrollFreeze(0, 1);

                        for (s32 col = 0; col < column_count; ++col) {
                            ImGui::TableSetupColumn(table_columns[col]);
                        }

                        ImGui::TableHeadersRow();

                        for (app_AddressTableEntry *ptr = app_addr_table_next_in_pointer_chain(entry);
                             ptr != entry->as_pointer_chain.sentinel;
                             ptr = app_addr_table_next_in_pointer_chain(ptr)) {
                            ASSERT(ptr);

                            ImGui::TableNextRow();

                            uintptr_t  address = app_addr_table_entry_get_address(ptr);
                            usize      offset  = app_addr_table_entry_get_offset(ptr);

                            scan_Value value = {};
                            b32 read_successful = app_addr_table_entry_try_get_value(ptr, &value);

                            ImGui::PushID(ptr); {
                                // Offset
                                ImGui::TableSetColumnIndex(0); {
                                    usize textbox_length = strlen(ptr->gui_input.offset_textbox);
                                    usize textbox_offset = 0;

                                    b32 offset_parse_ok = stn_parse_usize(
                                        ptr->gui_input.offset_textbox,
                                        textbox_length,
                                        &textbox_offset,
                                        10);

                                    if (ImGui::Button("<") && offset_parse_ok) {
                                        textbox_offset = sat_add_usize(textbox_offset, 1);
                                    }

                                    ImGui::SameLine();

                                    ImGui::InputText(
                                        "##offset_textbox",
                                        ptr->gui_input.offset_textbox,
                                        array_capacity(ptr->gui_input.offset_textbox));

                                    ImGui::SameLine();

                                    if (ImGui::Button(">") && offset_parse_ok) {
                                        textbox_offset = sat_sub_usize(textbox_offset, 1);
                                    }

                                    parsing_ok = parsing_ok && offset_parse_ok;

                                    // TODO: do this further up before the entry is updated
                                    if (offset_parse_ok) {
                                        ptr->as_pointer_chain.offset = textbox_offset;
                                    }
                                }

                                // Result
                                ImGui::TableSetColumnIndex(1); {
                                    const char *value_str = 0;

                                    if (read_successful) {
                                        value_str =
                                            str_format_cstr(gui->frame_arena, "%" PRIxPTR, value.pointer);
                                    } else {
                                        value_str = "???";
                                    }

                                    char *result_str = str_format_cstr(
                                        gui->frame_arena,
                                        "[%" PRIxPTR " + %" PRIuPTR "] = %s",
                                        address,
                                        offset,
                                        value_str
                                    );

                                    ImGui::TextUnformatted(result_str);
                                }
                            } ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }
                } ImGui::EndChild();

                // Offset buttons
                {
                    if (ImGui::Button("Add")) {
                        // TODO: allow inserting in middle
                        app_addr_table_entry_push_offset(&app->address_table, entry);
                    }

                }

                // Display the base address of the pointer chain, along with the value
                // found at that address.
                if (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER) {
                    if (ImGui::InputText(
                            "##base_address",
                            gui->edit_address_table_entry_popup.address_textbox,
                            array_capacity(gui->edit_address_table_entry_popup.address_textbox),
                            input_text_flags)) {
                        should_save_entry = true;
                    }

                    ImGui::SameLine();

                    char *value_str = 0;

                    scan_Value first_value = {};

                    // TODO: code duplication
                    if (app_addr_table_entry_try_get_value(entry, &first_value)) {
                        value_str =
                            str_format_cstr(gui->frame_arena, "%" PRIxPTR, first_value.pointer);
                    } else {
                        value_str = str_clone_cstr(gui->frame_arena, "???");
                    }

                    char *result_str = str_format_cstr(
                        gui->frame_arena,
                        "= %s",
                        value_str
                    );

                    ImGui::TextUnformatted(result_str);
                }
            }

            ImGui::BeginDisabled(!parsing_ok); {
                if (ImGui::Button("OK")) {
                    should_save_entry = true;
                }
            } ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                should_close_popup = true;
            }
        }

        // Handle closing and saving.
        {
            if (should_save_entry && ALWAYS(parsing_ok)) {
                should_close_popup = true;

                // Reallocate the entry in the address tables entry pool.
                app_AddressTableEntry *entry_copy = app_addr_table_entry_clone(app->address_table.pool, entry);

                if (existing_entry) {
                    // Commit the changes we have made to the copy of the entry.
                    app_action_edit_address_table_entry(app, existing_entry, entry_copy);
                } else {
                    app_action_push_address_table_entry(app, entry_copy);
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                should_save_entry = false;
                should_close_popup = true;
            }

            if (should_close_popup) {
                gui_popup_close(gui, GUI_POPUP_ADD_ADDRESS);
            }
        }

        ImGui::EndPopup();
    }
}

void gui__popup_address_watch(gui_State *gui)
{
    gui__set_initial_popup_size(0.75f, 0.75f);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    app_Watch *watch = gui->address_watch_popup.watch;

    if (watch) {
        const char *title = str_format_cstr(gui->frame_arena, "Address watch: %" PRIx64, watch->breakpoint_address);
        const char *popup_name = gui__popup_get_name(gui->frame_arena, GUI_POPUP_WATCH, title);

        if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
            ASSERT(watch);

            const char *table_columns[] = {"Count", "Address", "Instruction"};
            s32 column_count = array_capacity(table_columns);

            s32 table_flags = ImGuiTableFlags_Resizable
                | ImGuiTableFlags_BordersV
                | ImGuiTableFlags_Borders
                | ImGuiTableFlags_ScrollY
                | ImGuiTableFlags_SizingStretchSame;


            f32 instr_table_height_fraction = ImGui::GetContentRegionAvail().y * 0.7f;

            ImGui::BeginChild("##child_instr_table", {-FLT_MIN, instr_table_height_fraction}); {
                if (ImGui::BeginTable("#instruction_table", column_count, table_flags, {})) {
                    ImGui::TableSetupScrollFreeze(0, 1);

                    for (s32 col = 0; col < column_count; ++col) {
                        ImGui::TableSetupColumn(table_columns[col]);
                    }

                    ImGui::TableHeadersRow();

                    // s32 instr_count = list_count(&watch->triggered_instruction_list);

                    s32 selectable_flags = ImGuiSelectableFlags_SpanAllColumns;

                    for (app_WatchInstruction *instr = list_head(&watch->triggered_instruction_list);
                         instr;
                         instr = list_next(instr)) {
                        ImGui::TableNextRow();

                        b32 is_row_selected = gui->address_watch_popup.selected_instruction == instr;

                        const char *instr_id = str_format_cstr(gui->frame_arena, "instr_%" PRIxPTR, instr->start_address);

                        ImGui::PushID(instr_id); {
                            // Instruction trigger count
                            ImGui::TableSetColumnIndex(0); {
                                char *trigger_count_str = str_format_cstr(gui->frame_arena, "%" PRIu32, instr->trigger_count);

                                if (ImGui::Selectable(trigger_count_str, is_row_selected, selectable_flags)) {
                                    gui->address_watch_popup.selected_instruction = instr;
                                }
                            }

                            // Instruction address
                            ImGui::TableSetColumnIndex(1); {
                                char *address_str = str_format_cstr(gui->frame_arena, "%" PRIx64, instr->start_address);

                                if (ImGui::Selectable(address_str, is_row_selected, selectable_flags)) {
                                    gui->address_watch_popup.selected_instruction = instr;
                                }
                            }

                            // Instruction mnemonic
                            ImGui::TableSetColumnIndex(2); {
                                char *mnemonic_str = str_to_cstr(gui->frame_arena, instr->mnemonic);

                                if (ImGui::Selectable(mnemonic_str, is_row_selected, selectable_flags)) {
                                    gui->address_watch_popup.selected_instruction = instr;
                                }
                            }
                        } ImGui::PopID();
                    }
                } ImGui::EndTable();
            } ImGui::EndChild();

            ImGui::TextUnformatted("Registers last time instruction was hit:");

            // TODO: allow copying values of registers
            f32 regs_instr_child_height = -ImGui::GetFrameHeightWithSpacing();

            ImGui::BeginChild("##child_instr_regs", {-FLT_MIN, regs_instr_child_height}, ImGuiChildFlags_Borders); {
                app_WatchInstruction *selected_instr = gui->address_watch_popup.selected_instruction;
                b32 registers_are_valid = selected_instr
                    && ALWAYS(selected_instr->registers_after_execution.status == PROC_STATUS_OK);

                for (proc_Register reg = {}; reg < PROC_REG_COUNT; inc_enum(&reg)) {
                    // TODO: if debuggee is 32 bits, display 32 bit names
                    char *reg_name_str = proc_register_to_cstr(gui->frame_arena, reg);

                    char *reg_val_str = 0;
                    if (registers_are_valid) {
                        proc_RegisterValue reg_val = selected_instr->registers_after_execution.values[reg];

                        // TODO: enum for number base
                        b32 is_hex = gui->address_watch_popup.hexadecimal_register_display;
                        reg_val_str = proc_register_value_to_cstr(gui->frame_arena, reg_val, is_hex);
                    } else {
                        reg_val_str = str_clone_cstr(gui->frame_arena, "???");
                    }

                    char *reg_name_with_colon = str_format_cstr(gui->frame_arena, "%s:", reg_name_str);
                    char *registers_text = str_format_cstr(gui->frame_arena, "%-4s %-32s", reg_name_with_colon, reg_val_str);

                    ImGui::TextUnformatted(registers_text);

                    if (((s32)reg % 2) == 0) {
                        ImGui::SameLine();
                    }
                }
            } ImGui::EndChild();

            if (ImGui::Button("Stop")) {
                // TODO: implement stopping
                gui_popup_close(gui, GUI_POPUP_WATCH);
            }

            ImGui::SameLine();
            ImGui::Checkbox("Hexadecimal", &gui->address_watch_popup.hexadecimal_register_display);

            ImGui::EndPopup();
        }
    }
}


void gui_popup_display_active_popups(gui_State *gui, app_State *app)
{
    for (gui_Popup popup = {}; popup < GUI_POPUP_COUNT; inc_enum(&popup)) {
        if (gui->popup_states[popup]) {
            const char *name = gui__popup_get_name(gui->frame_arena, popup, 0);

            ImGui::OpenPopup(name);

            // Break after first open popup so that they're opened in sequence.
            // TODO: instead make them stack
            break;
        }

    }

    gui__popup_open_process(gui, app);
    gui__popup_edit_value(gui, app);
    gui__popup_edit_address_table_entry(gui, app);
    gui__popup_address_watch(gui);

    gui__generic_error_popup(gui, GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
        "Insufficient permissions to perform scan.");
    gui__generic_error_popup(gui, GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
        "Process died during scan.");
    gui__generic_error_popup(gui, GUI_POPUP_ERROR_WRITE_FAILED,
        "Failed to write to address.");

}

void gui_popup_error(gui_State *gui, app_Error error)
{
    BEGIN_EXHAUSTIVE_SWITCH;
    switch (error.kind) {
        case APP_ERROR_PROCESS_DIED_BEFORE_OR_DURING_SCAN: {
            gui__popup_open(gui, GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN);
        } break;

        case APP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN: {
            gui__popup_open(gui, GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN);
        } break;

        case APP_ERROR_WRITE_FAILED: {
            gui__popup_open(gui, GUI_POPUP_ERROR_WRITE_FAILED);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;
}
