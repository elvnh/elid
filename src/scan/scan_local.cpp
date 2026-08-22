#include "scan_local.h"

static scan_Value scan__load_value_at_address(void *address, scan_ValueType type)
{
    scan_Value result = {};

    usize type_size = scan_val_get_type_size(type);

    copy_mem(&result, address, 1, type_size);

    return result;
}


#define DBG_FIXED_MATCH_ARRAY_CAP 1024

scan_Result scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, uintptr_t base_address, usize stride)
{
    ASSERT(!scan_cmp_mode_is_comparative(cmp.mode));

    scan_Result result = {};
    result.matches = arena_alloc_array(arena, scan_Match, DBG_FIXED_MATCH_ARRAY_CAP); // TODO: don't use fixed size

    usize type_size = scan_val_get_type_size(cmp.value_type);

    usize offset = 0;

    while ((offset + type_size) <= memory_size) {
        void *local_address = ptr_byte_offset(memory, offset);

        scan_Value value_at_address = scan__load_value_at_address(local_address, cmp.value_type);

        if (scan_cmp_match(value_at_address, cmp)) {
            uintptr_t remote_address = (uintptr_t)base_address + offset;

            scan_Match match = {};
            match.value = value_at_address;
            match.remote_address = remote_address;

            ASSERT_ALWAYS(result.match_count < DBG_FIXED_MATCH_ARRAY_CAP);

            result.matches[result.match_count++] = match;
        }

        offset += stride;
    }

    return result;
}
