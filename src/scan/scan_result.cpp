#include "scan_result.h"

// Used when constructing scan_ResultView from scan_MatchList, when region is unknown.
global scan_RegionMatches g__dummy_region;

uintptr_t scan_address_from_match_details(scan_MatchDetails details)
{
    uintptr_t result = details.region_base_address + details.match->offset_from_base;

    return result;
}

scan_MatchDetails scan__match_details_from_match(scan_MatchList *matches, scan_RegionMatches *region, usize index)
{
    scan_MatchDetails result = {};
    result.match = &matches->match_data[index];
    result.history = &matches->match_history_data[index];
    result.region_base_address = region->region_base_address;
    result.containing_object = region->containing_object;

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
                scan_MatchDetails details = scan__match_details_from_match(matches, region, i);

                result.data[result.count++] = details;
            }
        }
    }

    ASSERT(result.count == scan_result.total_match_count);

    return result;
}

scan_ResultView scan_match_list_make_view(Arena *arena, scan_MatchList *matches)
{
    scan_ResultView result = {};
    result.data = arena_alloc_array(arena, scan_MatchDetails, matches->total_match_count);

    for (scan_MatchList *m = matches; m; m = list_next(m)) {
        for (usize i = 0; i < m->count; ++i) {
            result.data[result.count++] = scan__match_details_from_match(m, &g__dummy_region, i);
        }
    }

    return result;
}
