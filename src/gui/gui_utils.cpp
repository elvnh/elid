#include "gui_utils.h"

scan_Value_Maybe gui_try_parse_search_value(const char *input, scan_ValueType type, u32 base)
{
    scan_Value_Maybe result = {};

    usize length = strlen(input);

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            u32 u32_val = 0;
            s32 s32_val = 0;

            if (stn_parse_u32(input, length, &u32_val, base)) {
                result.value.int32 = u32_val;
                result.ok = true;
            } else if (stn_parse_s32(input, length, &s32_val, base)) {
                result.value.int32 = (u32)s32_val;
                result.ok = true;
            }
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            TODO("Parsing floats");
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);
        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

usize_Maybe gui_try_parse_address(const char *input, u32 base)
{
    usize_Maybe result = {};

    usize length = strlen(input);

    result.ok = stn_parse_usize(input, length, &result.value, base);

    return result;
}

scan_Comparison_Maybe gui_try_parse_scan_input(const char *input, scan_ValueType type, u32 base, scan_Mode mode)
{
    scan_Comparison_Maybe result = {};

    scan_Value_Maybe scan_value_opt = gui_try_parse_search_value(input, type, base);
    usize mode_arity = scan_mode_get_arity(mode);

    if (scan_value_opt.ok || (mode_arity == 0)) {
        scan_Value value = scan_value_opt.value;

        switch (mode) {
            case SCAN_MODE_EXACT: {
                result = SOME(scan_cmp_make_exact({type, value}));
            } break;

            case SCAN_MODE_UNKNOWN: {
                result = SOME(scan_cmp_make_unknown(type));
            } break;

            case SCAN_MODE_GREATER_THAN: {
                result = SOME(scan_cmp_make_greater_than({type, value}));
            } break;

            case SCAN_MODE_LESS_THAN: {
                result = SOME(scan_cmp_make_less_than({type, value}));
            } break;

            case SCAN_MODE_INCREASED: {
                result = SOME(scan_cmp_make_increased(type));
            } break;

            case SCAN_MODE_INCREASED_BY: {
                result = SOME(scan_cmp_make_increased_by({type, value}));
            } break;

            case SCAN_MODE_DECREASED: {
                result = SOME(scan_cmp_make_decreased(type));
            } break;

            case SCAN_MODE_DECREASED_BY: {
                result = SOME(scan_cmp_make_decreased_by({type, value}));
            } break;

            case SCAN_MODE_CHANGED: {
                result = SOME(scan_cmp_make_changed(type));
            } break;

            case SCAN_MODE_CHANGED_BY: {
                result = SOME(scan_cmp_make_changed_by({type, value}));
            } break;

            case SCAN_MODE_UNCHANGED: {
                result = SOME(scan_cmp_make_unchanged(type));
            } break;

            INVALID_CASE(SCAN_MODE_COUNT);
            INVALID_DEFAULT_CASE;
        }
    }

    return result;
}

char *gui_cstr_from_address(Arena *arena, uintptr_t address)
{
    char *result = str_format_cstr(arena, "%" PRIxPTR, address);

    return result;
}

char *gui_cstr_from_match_address(Arena *arena, scan_MatchDetails match_details, proc_ObjectList objects)
{
    ASSERT(objects.data);
    ASSERT(objects.count);

    char *result = 0;

    usize match_address = scan_address_from_match_details(match_details);

    if (match_details.containing_object.ok) {
        // This address is contained in library/executable itself, display as an offset.
        ASSERT(match_details.containing_object.value.base_address <= match_address);
        ASSERT(!str_is_null_or_empty(match_details.containing_object.value.name));

        proc_Object obj = match_details.containing_object.value;
        usize offset_from_object_base = match_address - obj.base_address;

        String object_name = {};

        if (proc_object_has_unique_filename(obj, objects)) {
            // If this object can be uniquely identified among all loaded objects
            // by its filename alone, it suffices to display the filename.
            object_name = path_filename(obj.name);
        } else {
            // Otherwise, we'll display the full path.
            object_name = obj.name;
        }

        result = str_format_cstr(arena, "\"" FMT_STR "\"" "+%" PRIxPTR,
            FMT_STR_ARG(object_name),
            offset_from_object_base);
    } else {
        // This is a regular address, display as normal.
        result = gui_cstr_from_address(arena, match_address);
    }

    return result;
}

char *gui_cstr_from_match_value(Arena *arena, scan_Value value, scan_ValueType value_type)
{
    char *result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (value_type) {
        case SCAN_VALUE_TYPE_INT32: {
            // TODO: allow controlling whether signed or unsigned
            result = str_format_cstr(arena, "%u", value.int32);
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            TODO("Formatting floats");
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);
        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

char *gui_cstr_from_value_type(Arena *arena, scan_ValueType type)
{
    char *result = 0;
    const char *str = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            str = "4 bytes";
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            str = "Float";
        } break;

        case SCAN_VALUE_TYPE_COUNT: {
            ASSERT(0);

            str = "<invalid>";
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    // NOTE: this copy is obviously unnecessary but I do it so that we can return a char * just like
    // from the other gui_cstr_from_* functions. These strings are so small anyways that it doesn't matter.
    result = str_clone_cstr(arena, str);

    return result;
}

scan_Settings gui_get_scan_settings(gui_State *gui)
{
    // TODO: get scan settings from current gui state
    scan_Settings result = {};
    result.required_alignment = 4;

    return result;
}

scan_ValueType gui_value_type_dropdown(gui_State *gui, const char *title, scan_ValueType current_type, f32 width)
{
    scan_ValueType result = current_type;

    if (title) {
        ImGui::TextUnformatted("Value type:");
        ImGui::SameLine();
    }

    s32 combo_flags = ImGuiComboFlags_HeightLargest;
    scan_ValueTypeList available_types = scan_val_type_get_list();
    char *value_type_preview = gui_cstr_from_value_type(gui->frame_arena, current_type);

    ImGui::SetNextItemWidth(width);

    if (ImGui::BeginCombo("##value_type", value_type_preview, combo_flags)) {
        for (usize i = 0; i < available_types.count; ++i) {
            scan_ValueType type = available_types.data[i];
            b32 is_type_selected = current_type == type;

            char *type_spelling = gui_cstr_from_value_type(gui->frame_arena, type);

            if (ImGui::Selectable(type_spelling, is_type_selected)) {
                result = type;
            }
        }

        ImGui::EndCombo();
    }

    return result;
}

void gui_push_no_selectable_highlight(void)
{
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0, 0, 0, 0));
}

void gui_pop_no_selectable_highlight(void)
{
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
}

void gui_set_initial_keyboard_focus_here(s32 offset)
{
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere(offset);
    }
}
