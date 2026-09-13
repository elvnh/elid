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


void scan_val_atomic_store(scan_Value *dst, scan_Value src)
{
    ASSERT((sizeof(scan_Value) == sizeof(u32)) && "Ensure the correct atomic operation is used");

    atomic_store_u32(&dst->int32, src.int32);
}

scan_Value scan_val_atomic_load(scan_Value *src)
{
    scan_Value result = {};
    result.int32 = atomic_load_u32(&src->int32);

    return result;
}

void scan_val_atomic_move(scan_Value *dst, scan_Value *src)
{
    scan_Value value = scan_val_atomic_load(src);
    scan_val_atomic_store(dst, value);
}
