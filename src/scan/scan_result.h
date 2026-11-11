#pragma once

// TODO: come up with a good size for this
#define SCAN_MATCH_LIST_SIZE 512

struct scan_Match {
    scan_Value value;
    uintptr_t  address;
};

struct scan_MatchHistory {
    scan_Value previous_value;
    scan_Value first_value;
};

// TODO: store exact scan results sparsely
/* An unrolled linked list of matches. */
struct scan_MatchList {
    scan_Match        match_data[SCAN_MATCH_LIST_SIZE];
    scan_MatchHistory match_history_data[SCAN_MATCH_LIST_SIZE];
    usize             count; // Number of matches in this list.

    usize             total_match_count; // Total number of matches in chain.

    scan_MatchList    *next;
};

/* The matches that were found in a single region of memory. */
struct scan_RegionMatches {
    usize               region_base_address;
    usize               region_size;
    proc_Object_Maybe   object_containing_region;

    scan_MatchList     *first_match_list;
    usize               match_count_in_region;

    scan_RegionMatches *next;
};

/* The complete result of a single scan. */
struct scan_Result {
    scan_Mode        scan_mode;
    scan_ValueType   value_type;
    proc_Status      errors; // TODO: rename to error_bitset

    memmi_Process    scanned_process;
    proc_ObjectList  process_objects;

    usize            total_match_count;

    struct {
        scan_RegionMatches *first;
        scan_RegionMatches *last;
        usize               list_node_count;

        // The sum of the sizes of all regions in which at least one match was found.
        usize               matching_regions_size_sum;
    } regions;
};

/* A non-owning view of a single match in a scan_MatchList. */
struct scan_MatchView {
    scan_Match         *match;
    scan_MatchHistory  *history;
    proc_Object_Maybe   containing_object;
};

/* A flat array of scan_MatchView, sorted by memory address. */
struct scan_ResultView {
    scan_MatchView *data;
    usize count;
};

scan_ResultView scan_result_view_from_result(Arena *arena, scan_Result scan_result);
scan_ResultView scan_result_view_from_match_list(Arena *arena, scan_MatchList *matches);
b32             scan_result_view_eq(scan_ResultView lhs, scan_ResultView rhs);
