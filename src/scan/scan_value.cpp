#include "scan_value.h"

// TODO: reduce code duplication
// TODO: unit testing

usize scan_value_get_type_size(scan_Type type)
{
    usize result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32:
        case SCAN_TYPE_FLOAT32: {
            result = 4;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_value_op_add(scan_Type type, scan_Value a, scan_Value b)
{
    scan_Value result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result.int32 = a.int32 + b.int32;
        } break;

        case SCAN_TYPE_FLOAT32: {
            result.float32 = a.float32 + b.float32;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_value_op_sub(scan_Type type, scan_Value a, scan_Value b)
{
    scan_Value result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result.int32 = a.int32 - b.int32;
        } break;

        case SCAN_TYPE_FLOAT32: {
            result.float32 = a.float32 - b.float32;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}


b32 scan_value_cmp_eq(scan_Type type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result = (a.int32 == b.int32);
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            TODO("Unimplemented");
            // result = (a.float32 == b.float32);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_value_cmp_gt(scan_Type type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            // We have to compare these both as signed and unsigned integers, as we don't
            // know which signedness the values have. This means we'll receive some false
            // positives/negatives.
            result = (a.int32 > b.int32) || (wrap_u32_to_s32(a.int32) > wrap_u32_to_s32(b.int32));
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            TODO("Unimplemented");
            // result = (a.float32 > b.float32);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_value_cmp_lt(scan_Type type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            // We have to compare these both as signed and unsigned integers, as we don't
            // know which signedness the values have. This means we'll receive some false
            // positives/negatives.
            result = (a.int32 < b.int32) || (wrap_u32_to_s32(a.int32) < wrap_u32_to_s32(b.int32));
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            TODO("Unimplemented");
            // result = (a.float32 < b.float32);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_value_make_int32(u32 value)
{
    scan_Value result = {};
    result.int32 = value;

    return result;
}
