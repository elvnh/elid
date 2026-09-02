#include "scan_result.h"

static scan_MatchDetails scan__region_matches_at(scan_RegionMatches *matches, usize at)
{
    ASSERT(at < matches->match_count_in_region);
    scan_MatchDetails result = {};

    scan_MatchList *current = matches->head;

    usize i = 0;
    while (current && (i + current->count) <= at) {
        i += current->count;
        current = list_next(current);
    }

    ASSERT(current);

    usize index = at - i;
    ASSERT(index < current->count);

    scan_Match *match = &current->match_data[index];
    scan_MatchHistory *hist = &current->match_history_data[index];
    result.match = match;
    result.history = hist;

    result.region_base_address = matches->region_base_address;
    result.containing_object = matches->containing_object;

    return result;
}

scan_MatchDetails scan_result_at(scan_Result *matches, usize at)
{
    ASSERT(at < matches->total_match_count);

    scan_MatchDetails result = {};

    scan_RegionMatches *region_matches = matches->first;

    usize i = 0;
    while (region_matches && (i + region_matches->match_count_in_region) <= at) {

        i += region_matches->match_count_in_region;
        region_matches = list_next(region_matches);
    }

    if (ALWAYS(region_matches)) {
        usize index = at - i;
        result = scan__region_matches_at(region_matches, index);
    }

    return result;
}

uintptr_t scan_address_from_match_details(scan_MatchDetails details)
{
    uintptr_t result = details.region_base_address + details.match->offset_from_base;

    return result;
}
