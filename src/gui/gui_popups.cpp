#include "gui_popups.h"

#define GUI_OPEN_PROC_POPUP_TITLE "Open process"

char *gui__error_popup_name(Arena *arena, gui_Popup popup)
{
    char *result = str_format_cstr(arena, "Error##%d", (s32)popup);

    return result;
}

void gui_popup_open(Arena *arena, gui_Popup popup)
{
    BEGIN_EXHAUSTIVE_SWITCH;
    switch (popup) {
        case GUI_POPUP_OPEN_PROCESS: {
            ImGui::OpenPopup(GUI_OPEN_PROC_POPUP_TITLE);
        } break;

        case GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN: {
            ImGui::OpenPopup(gui__error_popup_name(arena, GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN));
        } break;

        case GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN: {
            ImGui::OpenPopup(gui__error_popup_name(arena, GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN));
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;
}

void gui_popup_close()
{
    ImGui::CloseCurrentPopup();
}

static void gui__popup_open_process(gui_State *gui, app_State *app)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::BeginPopupModal(GUI_OPEN_PROC_POPUP_TITLE, 0, popup_flags)) {
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
            gui_popup_close();
        }

        ImGui::EndPopup();
    }
}

static void gui__generic_error_popup(Arena *arena, gui_Popup popup, const char *text)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 root_window_size = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({root_window_size.x * 0.75f, root_window_size.y * 0.75f}, ImGuiCond_Appearing);

    s32 popup_flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    char *popup_name = gui__error_popup_name(arena, popup);

    if (ImGui::BeginPopupModal(popup_name, 0, popup_flags)) {
        ImGui::TextUnformatted(text);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            gui_popup_close();
        }

        ImGui::EndPopup();
    }
}

void gui_popup_display_active_popups(gui_State *gui, app_State *app)
{
    gui__popup_open_process(gui, app);
    gui__generic_error_popup(app->frame_arena, GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
        "Insufficient permissions to perform scan.");
    gui__generic_error_popup(app->frame_arena, GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
        "Process died during scan.");
}
