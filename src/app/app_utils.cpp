#include "app_utils.h"

b32 app_read_single_value_in_process(memmi_Process proc, uintptr_t address, scan_ValueType type, scan_Value *out)
{
    scan_Value value = {};
    usize value_size = scan_val_type_get_size(type);

    memmi_ReadMemory read_mem_result = memmi_read_memory(proc, &value, address, value_size);

    b32 result = read_mem_result.status == MEMMI_OK;

    if (result) {
        *out = value;
    }

    return result;
}
