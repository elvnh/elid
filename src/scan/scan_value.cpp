#include "scan_value.h"

// TODO: reduce code duplication

usize scan_val_type_get_size(scan_ValueType type)
{
    usize result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32:
        case SCAN_VALUE_TYPE_FLOAT32: {
            result = 4;
        } break;

        case SCAN_VALUE_TYPE_COUNT: {
            ASSERT(0);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_ValueTypeList scan_val_type_get_list()
{
    scan_ValueTypeList result = {};

    for (scan_ValueType type = {}; type < SCAN_VALUE_TYPE_COUNT; inc_enum(&type)) {
        result.data[result.count++] = type;
    }

    return result;
}

const char *scan_val_type_to_cstr(scan_ValueType type)
{
    const char *result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            result = "4 bytes";
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            result = "Float";
        } break;

        case SCAN_VALUE_TYPE_COUNT: {
            ASSERT_ALWAYS(0);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_val_make_int32(u32 value)
{
    scan_Value result = {};
    result.int32 = value;

    return result;
}

scan_Value scan_val_add(scan_ValueType type, scan_Value a, scan_Value b)
{
    scan_Value result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            result.int32 = a.int32 + b.int32;
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            result.float32 = a.float32 + b.float32;
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_val_sub(scan_ValueType type, scan_Value a, scan_Value b)
{
    scan_Value result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            result.int32 = a.int32 - b.int32;
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            result.float32 = a.float32 - b.float32;
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_val_eq(scan_ValueType type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            result = (a.int32 == b.int32);
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            TODO("Unimplemented");
            // result = (a.float32 == b.float32);
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_val_gt(scan_ValueType type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_VALUE_TYPE_INT32: {
            // We have to compare these both as signed and unsigned integers, as we don't
            // know which signedness the values have. This means we'll receive some false
            // positives/negatives.
            result = (a.int32 > b.int32) || (wrap_u32_to_s32(a.int32) > wrap_u32_to_s32(b.int32));
        } break;

        case SCAN_VALUE_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            TODO("Unimplemented");
            // result = (a.float32 > b.float32);
        } break;

        INVALID_CASE(SCAN_VALUE_TYPE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_val_lt(scan_ValueType type, scan_Value a, scan_Value b)
{
    b32 result = !scan_val_gt(type, a, b) && !scan_val_eq(type, a, b);

    return result;
}

