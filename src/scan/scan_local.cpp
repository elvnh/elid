#include "scan_local.h"

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, uintptr_t base_address,
    scan_Comparison cmp, usize alignment)
{
    // TODO: now that we're providing the base address, we may as well store the
    // absolute address in scan_Match

    ASSERT(!scan_mode_is_comparative(cmp.mode));
    ASSERT(is_pow2(alignment)); // TODO: don't require pow2 alignment

    ASSERT_ALWAYS(is_aligned(base_address, alignment));

    scan_MatchList *result = arena_alloc_struct(arena, scan_MatchList);

    usize type_size = scan_val_type_get_size(cmp.value_type);

    usize offset = 0;

    while ((offset + type_size) <= mem_size) {
        void *local_address = ptr_byte_offset(mem, offset);

        scan_Value value_at_address = scan_val_bit_cast(local_address, cmp.value_type);

        if (scan_cmp_match(value_at_address, cmp)) {
            scan_Match match = {};
            match.value = value_at_address;
            match.address = base_address + offset;

            // There is no history yet as this is the first scan.
            scan_MatchHistory match_history = {};
            match_history.first_value = match.value;
            match_history.previous_value = match.value;

            scan_match_list_push(arena, &result, match, match_history);
        }

        offset += alignment;
    }

    return result;
}

scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, uintptr_t base_address, scan_Comparison cmp,
    scan_MatchList *prev_matches)
{
    scan_MatchList *result = arena_alloc_struct(arena, scan_MatchList);

    for (scan_MatchList *match_list = prev_matches; match_list; match_list = list_next(match_list)) {
        for (usize i = 0; i < match_list->count; ++i) {
            scan_Match prev_match = match_list->match_data[i];
            scan_MatchHistory history = match_list->match_history_data[i];
            history.previous_value = prev_match.value;

            ASSERT(prev_match.address >= base_address);
            usize offset_from_base = prev_match.address - base_address;
            ASSERT(offset_from_base < mem_size);

            void *address = ptr_byte_offset(mem, offset_from_base);
            scan_Value current_value = scan_val_bit_cast(address, cmp.value_type);
            scan_Value prev_value = prev_match.value;

            if (scan_cmp_match_with_previous(current_value, cmp, prev_value)) {
                scan_Match new_match = {};
                new_match.value = current_value;
                new_match.address = prev_match.address;

                scan_match_list_push(arena, &result, new_match, history);
            }
        }
    }

    return result;
}
