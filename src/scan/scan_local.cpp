#include "scan_local.h"

static scan_Value scan__load_value_at_address(void *address, scan_ValueType type)
{
    scan_Value result = {};

    usize type_size = scan_val_get_type_size(type);

    copy_mem(&result, address, 1, type_size);

    return result;
}

static scan_MatchList *scan__alloc_match_list(Arena *arena, scan_MatchList *next)
{
    scan_MatchList *result = arena_alloc_struct(arena, scan_MatchList);
    result->next = next;

    return result;
}

scan_MatchList *scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, usize stride)
{
    ASSERT(!scan_cmp_mode_is_comparative(cmp.mode));

    scan_MatchList *result = scan__alloc_match_list(arena, 0);

    usize type_size = scan_val_get_type_size(cmp.value_type);

    usize offset = 0;

    while ((offset + type_size) <= memory_size) {
        void *local_address = ptr_byte_offset(memory, offset);

        scan_Value value_at_address = scan__load_value_at_address(local_address, cmp.value_type);

        if (scan_cmp_match(value_at_address, cmp)) {
            ASSERT(result->match_count <= ARRAY_COUNT(result->matches));

            if (result->match_count == ARRAY_COUNT(result->matches)) {
                result = scan__alloc_match_list(arena, result);
            }

            scan_Match match = {};
            match.value = value_at_address;
            match.offset_from_base = offset;

            result->matches[result->match_count++] = match;
        }

        offset += stride;
    }

    return result;
}
