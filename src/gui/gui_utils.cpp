#include "gui_utils.h"

scan_Value_Maybe gui_try_parse_search_value(const char *input, scan_ValueType type, u32 base)
{
    scan_Value_Maybe result = {};

    size_t length = strlen(input);

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
