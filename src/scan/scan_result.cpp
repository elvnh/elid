#include "scan_result.h"

// Used when constructing scan_ResultView from scan_MatchList, when region is unknown.
global scan_RegionMatches g__dummy_region;

scan_MatchView scan__match_details_from_match(scan_MatchList *matches, scan_RegionMatches *region, usize index)
{
    scan_MatchView result = {};
    result.match = &matches->match_data[index];
    result.history = &matches->match_history_data[index];
    result.region_base_address = region->region_base_address;
    result.containing_object = region->object_containing_region;

    return result;
}

int scan__result_view_sort_cmp(const void *a, const void *b)
{
    scan_MatchView details_a = *(scan_MatchView *)a;
    scan_MatchView details_b = *(scan_MatchView *)b;

    uintptr_t address_a = details_a.match->address;
    uintptr_t address_b = details_b.match->address;

    int result = 0;

    if (address_a < address_b) {
        result = -1;
    } else if (address_a > address_b) {
        result = 2;
    }

    return result;
}

void scan__result_view_sort(scan_ResultView *results)
{
    if (results->count > 0) {
        ASSERT(results->data);
        qsort(results->data, results->count, sizeof(*results->data), scan__result_view_sort_cmp);
    }

    #if DEBUG_BUILD
    {
        if (results->count > 0) {
            for (usize i = 0; i < results->count - 1; ++i) {
                uintptr_t this_addr = results->data[i].match->address;
                uintptr_t next_addr = results->data[i + 1].match->address;

                ASSERT((this_addr != next_addr) && "Duplicate address found");
                ASSERT(this_addr < next_addr);
            }
        }
    }
    #endif
}

scan_ResultView scan_result_view_from_result(Arena *arena, scan_Result scan_result)
{
    // ASSERT((scan_result.total_match_count < 50000) && "Only call this with a reasonable number of matches");

    scan_ResultView result = {};

    if (scan_result.total_match_count > 0) {
        result.data = arena_alloc_array(arena, scan_MatchView, scan_result.total_match_count);

        for (scan_RegionMatches *region = list_head(&scan_result.regions); region; region = list_next(region)) {
            for (scan_MatchList *matches = region->first_match_list; matches; matches = list_next(matches)) {
                for (usize i = 0; i < matches->count; ++i) {
                    scan_MatchView details = scan__match_details_from_match(matches, region, i);

                    ASSERT(result.count < scan_result.total_match_count);

                    result.data[result.count++] = details;
                }
            }
        }
    }

    ASSERT(result.count == scan_result.total_match_count);

    scan__result_view_sort(&result);

    return result;
}

scan_ResultView scan_result_view_from_match_list(Arena *arena, scan_MatchList *matches)
{
    scan_ResultView result = {};
    result.data = arena_alloc_array(arena, scan_MatchView, matches->total_match_count);

    for (scan_MatchList *m = matches; m; m = list_next(m)) {
        for (usize i = 0; i < m->count; ++i) {
            result.data[result.count++] = scan__match_details_from_match(m, &g__dummy_region, i);
        }
    }

    scan__result_view_sort(&result);

    return result;
}

b32 scan_result_view_eq(scan_ResultView lhs, scan_ResultView rhs)
{
    b32 result = lhs.count == rhs.count;

    for (usize i = 0; (i < lhs.count) && result; ++i) {
        scan_MatchView a = lhs.data[i];
        scan_MatchView b = rhs.data[i];

        result = (a.match->address == b.match->address)
            && scan_val_eq(SCAN_VALUE_TYPE_INT32, a.match->value, b.match->value)
            && scan_val_eq(SCAN_VALUE_TYPE_INT32, a.history->previous_value, b.history->previous_value)
            && scan_val_eq(SCAN_VALUE_TYPE_INT32, a.history->first_value, b.history->first_value);
    }

    return result;
}
