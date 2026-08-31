#include "gui_popups.h"

const char *gui__popup_get_name(Arena *arena, gui_Popup popup, const char *title)
{
    if (!title) {
        title = "";
    }

    // Only the popup enum will contribute to the ID, meaning we can provide different titles
    // when opening vs when defining the popup window.
    const char *id_str = str_format_cstr(arena, "%d", (s32)popup);
    ImGuiID id = ImGui::GetID(id_str);

    const char *result = str_format_cstr(arena, "%s###%u", title, id);

    return result;
}

void gui_popup_open(gui_State *gui, gui_Popup popup)
{
    gui->popup_states[popup] = true;
}

void gui_popup_close(gui_State *gui, gui_Popup popup)
{
    gui->popup_states[popup] = false;
    ImGui::CloseCurrentPopup();
}

static void gui__popup_open_process(gui_State *gui, app_State *app)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    const char *name = gui__popup_get_name(app->frame_arena, GUI_POPUP_OPEN_PROCESS, "Open process");

    if (ImGui::BeginPopupModal(name, 0, popup_flags)) {
        s32 process_count = usize_to_s32(app->running_process_list.count);
        b32 item_is_highlighted = false;

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

                if (ImGui::IsItemHovered()) {
                    item_is_highlighted = true;
                }

                ImGui::PopID();
            }

            ImGui::EndListBox();
        }

        // TODO: can selected index be a local variable?
        s32 selected_index = gui->process_list_popup.selected_index;
        b32 is_valid_index = (selected_index >= 0) && (selected_index < process_count);
        b32 has_double_clicked = item_is_highlighted && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

        b32 should_close = false;

        if ((ImGui::Button("Open") || has_double_clicked) && is_valid_index) {
            memmi_ProcessInfo clicked_process = app->running_process_list.data[selected_index];

            should_close = app_action_open_process(app, clicked_process.pid);
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            should_close = true;
        }

        if (should_close) {
            gui_popup_close(gui, GUI_POPUP_OPEN_PROCESS);
        }

        ImGui::EndPopup();
    }
}

static void gui__generic_error_popup(Arena *arena, gui_State *gui, gui_Popup popup, const char *text)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    const char *popup_name = gui__popup_get_name(arena, popup, "Error");

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
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    const char *title = str_format_cstr(app->frame_arena, "Edit value at %" PRIxPTR, gui->edit_value_popup.edited_address);
    const char *popup_name = gui__popup_get_name(app->frame_arena, GUI_POPUP_EDIT_VALUE, title);

    if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
        ImGui::InputText(
            "##edit_value",
            gui->edit_value_popup.value_textbox,
            array_capacity(gui->edit_value_popup.value_textbox)
        );

        ImGui::Separator();

        b32 should_close = false;

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            should_close = true;
            TODO("Edit value");
        }

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

void gui_popup_display_active_popups(gui_State *gui, app_State *app)
{
    for (gui_Popup popup = {}; popup < GUI_POPUP_COUNT; inc_enum(&popup)) {
        if (gui->popup_states[popup]) {
            const char *name = gui__popup_get_name(app->frame_arena, popup, 0);

            ImGui::OpenPopup(name);
        }

    }

    gui__popup_open_process(gui, app);
    gui__generic_error_popup(app->frame_arena, gui, GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
        "Insufficient permissions to perform scan.");
    gui__generic_error_popup(app->frame_arena, gui, GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
        "Process died during scan.");
    gui__popup_edit_value(gui, app);
}
