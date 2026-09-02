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

scan_Comparison_Maybe gui_try_parse_scan_input(const char *input, scan_ValueType type, u32 base, scan_Mode mode)
{
    scan_Comparison_Maybe result = {};

    scan_Value_Maybe scan_value_opt = gui_try_parse_search_value(input, type, base);
    usize mode_arity = scan_mode_get_arity(mode);

    if (scan_value_opt.ok || (mode_arity == 0)) {
        scan_Value value = scan_value_opt.value;

        switch (mode) {
            case SCAN_MODE_EXACT: {
                result = SOME(scan_cmp_make_exact(type, value));
            } break;

            case SCAN_MODE_UNKNOWN: {
                result = SOME(scan_cmp_make_unknown(type));
            } break;

            case SCAN_MODE_GREATER_THAN: {
                result = SOME(scan_cmp_make_greater_than(type, value));
            } break;

            case SCAN_MODE_LESS_THAN: {
                result = SOME(scan_cmp_make_less_than(type, value));
            } break;

            case SCAN_MODE_INCREASED: {
                result = SOME(scan_cmp_make_increased(type));
            } break;

            case SCAN_MODE_INCREASED_BY: {
                result = SOME(scan_cmp_make_increased_by(type, value));
            } break;

            case SCAN_MODE_DECREASED: {
                result = SOME(scan_cmp_make_decreased(type));
            } break;

            case SCAN_MODE_DECREASED_BY: {
                result = SOME(scan_cmp_make_decreased_by(type, value));
            } break;

            case SCAN_MODE_CHANGED: {
                result = SOME(scan_cmp_make_changed(type));
            } break;

            case SCAN_MODE_CHANGED_BY: {
                result = SOME(scan_cmp_make_changed_by(type, value));
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
            object_name = path_filename(obj.name);
        } else {
            object_name = obj.name;
        }

        result = str_format_cstr(arena, "\"" FMT_STR "\"" "+%" PRIxPTR,
            FMT_STR_ARG(object_name),
            offset_from_object_base);
    } else {
        // This is a regular address, display as normal.
        result = str_format_cstr(arena, "%" PRIxPTR, match_address);
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

scan_Settings gui_get_scan_settings(gui_State *gui)
{
    // TODO: get scan settings from current gui state
    scan_Settings result = {};

    return result;
}
