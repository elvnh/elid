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
b32        scan_value_equal(scan_Type type, scan_Value a, scan_Value b);
b32        scan_value_greater_than(scan_Type type, scan_Value a, scan_Value b);
scan_Value scan_value_add(scan_Type type, scan_Value a, scan_Value b);
