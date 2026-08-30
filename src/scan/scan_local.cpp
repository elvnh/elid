#include "scan_local.h"

static scan_Value scan__load_value_at_address(void *address, scan_ValueType type)
{
    scan_Value result = {};

    usize type_size = scan_val_type_get_size(type);

    copy_mem(&result, address, 1, type_size);

    return result;
}

static scan_MatchList *scan__alloc_match_list(Arena *arena)
{
    scan_MatchList *result = arena_alloc_struct(arena, scan_MatchList);
    result->first = result;
    result->last = result;

    return result;
}

static void scan__push_match(Arena *arena, scan_MatchList *list, scan_Match match)
{
    scan_MatchList *current = list->last;

    ASSERT(current->count <= ARRAY_COUNT(current->data));

    if (current->count == ARRAY_COUNT(current->data)) {
        current = scan__alloc_match_list(arena);
        sll_push_back(list, current);
    }

    current->data[current->count++] = match;

    ++list->total_match_count;
}

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, usize stride)
{
    ASSERT(!scan_mode_is_comparative(cmp.mode));

    scan_MatchList *result = scan__alloc_match_list(arena);

    usize type_size = scan_val_type_get_size(cmp.value_type);

    usize offset = 0;

    while ((offset + type_size) <= mem_size) {
        void *local_address = ptr_byte_offset(mem, offset);

        scan_Value value_at_address = scan__load_value_at_address(local_address, cmp.value_type);

        if (scan_cmp_match(value_at_address, cmp)) {
            scan_Match match = {};
            match.value = value_at_address;
            match.offset_from_base = offset;

            scan__push_match(arena, result, match);
        }

        offset += stride;
    }

    return result;
}

scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, scan_MatchList *prev_matches)
{
    scan_MatchList *result = scan__alloc_match_list(arena);

    for (scan_MatchList *match_list = prev_matches; match_list; match_list = list_next(match_list)) {
        for (usize i = 0; i < match_list->count; ++i) {
            scan_Match prev_match = match_list->data[i];
            ASSERT(prev_match.offset_from_base < mem_size);

            void *address = ptr_byte_offset(mem, prev_match.offset_from_base);
            scan_Value current_value = scan__load_value_at_address(address, cmp.value_type);
            scan_Value prev_value = prev_match.value;

            if (scan_cmp_match_with_previous(current_value, cmp, prev_value)) {
                scan_Match new_match = {};
                new_match.value = current_value;
                new_match.offset_from_base = prev_match.offset_from_base;

                scan__push_match(arena, result, new_match);
            }
        }
    }

    return result;
}
