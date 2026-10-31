#include "scan_result.h"

#if DEBUG_BUILD
#    define SCAN_DBG_ASSERT_VIEW_IS_SORTED(v)                           \
    do {                                                                \
        if ((v).count > 0) {                                            \
            for (usize i = 0; i < (v).count - 1; ++i) {                 \
                uintptr_t DBG_this_addr =                               \
                    scan_address_from_match_details((v).data[i]);       \
                uintptr_t DBG_next_addr =                               \
                    scan_address_from_match_details((v).data[i + 1]);   \
                                                                        \
                ASSERT(DBG_this_addr < DBG_next_addr);                  \
            }                                                           \
        }                                                               \
    } while (0)
#else
#    define SCAN_DBG_ASSERT_VIEW_IS_SORTED(v)
#endif

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

int scan__result_view_sort_cmp(const void *a, const void *b)
{
    scan_MatchDetails details_a = *(scan_MatchDetails *)a;
    scan_MatchDetails details_b = *(scan_MatchDetails *)b;

    uintptr_t address_a = scan_address_from_match_details(details_a);
    uintptr_t address_b = scan_address_from_match_details(details_b);

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
}

scan_ResultView scan_result_make_view(Arena *arena, scan_Result scan_result)
{
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

    scan__result_view_sort(&result);

    SCAN_DBG_ASSERT_VIEW_IS_SORTED(result);

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

    scan__result_view_sort(&result);

    SCAN_DBG_ASSERT_VIEW_IS_SORTED(result);

    return result;
}
