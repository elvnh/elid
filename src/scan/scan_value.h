#pragma once

enum scan_Type {
    SCAN_TYPE_INT32,
    SCAN_TYPE_FLOAT32,
};

union scan_Value {
    u32 int32;
    f32 float32;
};

usize      scan_value_get_type_size(scan_Type type);
scan_Value scan_value_make_int32(u32 value);
scan_Value scan_value_op_add(scan_Type type, scan_Value a, scan_Value b);
b32        scan_value_cmp_eq(scan_Type type, scan_Value a, scan_Value b);
b32        scan_value_cmp_gt(scan_Type type, scan_Value a, scan_Value b);
