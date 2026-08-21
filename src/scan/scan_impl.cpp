#include "scan_impl.h"

static scan_Value scan__load_value_at_address(void *address, scan_Type type)
{
    scan_Value result = {};

    usize type_size = scan_value_get_type_size(type);

    copy_mem(&result, address, 1, type_size);

    return result;
}

static b32 scan__compare_values(scan_Parameters scan_params, scan_Value current_value)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (scan_params.mode) {
        // Non-comparative scan modes
        case SCAN_MODE_EXACT: {
            result = scan_value_equal(scan_params.type, current_value, scan_params.args[0]);
        } break;

        case SCAN_MODE_GREATER_THAN: {
            result = scan_value_greater_than(scan_params.type, current_value, scan_params.args[0]);
        } break;

        // Comparative scan modes
        case SCAN_MODE_CHANGED: {
            scan_Value prev_value = scan_params.args[0];
            result = !scan_value_equal(scan_params.type, current_value, prev_value);
        } break;

        case SCAN_MODE_CHANGED_BY: {
            scan_Value prev_value = scan_params.args[0];
            scan_Value changed_by = scan_params.args[1];
            scan_Value expected_value = scan_value_add(scan_params.type, prev_value, changed_by);

            result = scan_value_equal(scan_params.type, current_value, expected_value);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Result scan_first(void *memory, usize memory_size, scan_Parameters params)
{
    // TODO: assert that this is a non-comparative scan mode
    TODO("Unimplemented");

    usize type_size = scan_value_get_type_size(params.type);

    usize offset = 0;
    usize stride = 1; // TODO: fast scan

    while ((offset + type_size) <= memory_size) {
        void *address = ptr_byte_offset(memory, offset);

        scan_Value value_at_address = scan__load_value_at_address(address, params.type);

        if (scan__compare_values(params, value_at_address)) {
            TODO("save match");
        }

        offset += stride;
    }

    return {};
}
