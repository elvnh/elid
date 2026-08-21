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

b32 scan_value_equal(scan_Type type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result = (a.int32 == b.int32);
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            result = (a.float32 == b.float32);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

b32 scan_value_greater_than(scan_Type type, scan_Value a, scan_Value b)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result = (a.int32 > b.int32);
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            result = (a.float32 > b.float32);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Value scan_value_add(scan_Type type, scan_Value a, scan_Value b)
{
    scan_Value result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (type) {
        case SCAN_TYPE_INT32: {
            result.int32 = a.int32 + b.int32;
        } break;

        case SCAN_TYPE_FLOAT32: {
            // TODO: should we compare non-exactly for floats?
            result.float32 = a.float32 + b.float32;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}
