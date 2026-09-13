#pragma once

enum scan_ValueType {
    SCAN_VALUE_TYPE_INT32,
    SCAN_VALUE_TYPE_FLOAT32,

    SCAN_VALUE_TYPE_COUNT
};

struct scan_ValueTypeList {
    scan_ValueType data[SCAN_VALUE_TYPE_COUNT];
    usize count;
};

union scan_Value {
    u32 int32;
    f32 float32;
};

DEFINE_MAYBE_TYPE(scan_Value);

usize               scan_val_type_get_size(scan_ValueType type);
scan_ValueTypeList  scan_val_type_get_list();
scan_Value          scan_val_make_int32(u32 value);
scan_Value          scan_val_add(scan_ValueType type, scan_Value a, scan_Value b);
scan_Value          scan_val_sub(scan_ValueType type, scan_Value a, scan_Value b);
b32                 scan_val_eq(scan_ValueType type, scan_Value a, scan_Value b);
b32                 scan_val_gt(scan_ValueType type, scan_Value a, scan_Value b);
b32                 scan_val_lt(scan_ValueType type, scan_Value a, scan_Value b);
void                scan_val_atomic_store(scan_Value *dst, scan_Value src);
scan_Value          scan_val_atomic_load(scan_Value *src);
void                scan_val_atomic_move(scan_Value *dst, scan_Value *src);
