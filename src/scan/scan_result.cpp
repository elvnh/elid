#include "scan_result.h"

uintptr_t scan_address_from_match_details(scan_MatchDetails details)
{
    uintptr_t result = details.region_base_address + details.match->offset_from_base;

    return result;
}

scan_ResultView scan_result_make_view(Arena *arena, scan_Result scan_result)
{
    // TODO: sort by address
    ASSERT((scan_result.total_match_count < 50000) && "Only call this with a reasonable number of matches");

    scan_ResultView result = {};
    result.data = arena_alloc_array(arena, scan_MatchDetails, scan_result.total_match_count);

    for (scan_RegionMatches *region = list_head(&scan_result); region; region = list_next(region)) {
        for (scan_MatchList *matches = region->head; matches; matches = list_next(matches)) {
            for (usize i = 0; i < matches->count; ++i) {
                scan_MatchDetails details = {};
                details.match = &matches->match_data[i];
                details.history = &matches->match_history_data[i];
                details.region_base_address = region->region_base_address;
                details.containing_object = region->containing_object;

                result.data[result.count++] = details;
            }
        }
    }

    ASSERT(result.count == scan_result.total_match_count);

    return result;
}
