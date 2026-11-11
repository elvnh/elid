#pragma once

// TODO: come up with a good size for this
#define SCAN_MATCH_LIST_SIZE 512

struct scan_Match {
    scan_Value value;
    usize      offset_from_base;
};

struct scan_MatchHistory {
    scan_Value previous_value;
    scan_Value first_value;
};

// TODO: rename
struct scan_MatchDetails {
    scan_Match         *match;
    scan_MatchHistory  *history;
    uintptr_t           region_base_address;
    proc_Object_Maybe   containing_object;
};

// TODO: store exact scan results sparsely
struct scan_MatchList {
    scan_Match        match_data[SCAN_MATCH_LIST_SIZE];
    scan_MatchHistory match_history_data[SCAN_MATCH_LIST_SIZE];
    usize count; // Number of matches in this list.

    usize total_match_count; // Total number of matches in chain.

    scan_MatchList *next;
};

struct scan_RegionMatches {
    // TODO: just store proc_Region here?
    usize  region_base_address;
    usize  region_size;

    proc_Object_Maybe containing_object;

    usize match_count_in_region;

    // TODO: rename
    scan_MatchList *head;

    scan_RegionMatches *next;
    scan_RegionMatches *last;
};

struct scan_Result {
    scan_Mode           scan_mode;
    scan_ValueType      value_type;
    proc_Status         errors; // TODO: rename to error_bitset

    memmi_Process       scanned_process;
    proc_ObjectList     process_objects;

    usize               total_match_count;

    // The sum of the sizes of all regions in which at least one match was found.
    usize               matching_regions_size_sum;

    // TODO: pack into struct
    scan_RegionMatches *first;
    scan_RegionMatches *last;
    usize               list_node_count;
};

struct scan_ResultView {
    scan_MatchDetails *data;
    usize count;
};

// TODO: scan_view_from_result
scan_ResultView scan_result_make_view(Arena *arena, scan_Result scan_result);
uintptr_t       scan_address_from_match_details(scan_MatchDetails details);
// TODO: scan_view_from_match_list
scan_ResultView scan_match_list_make_view(Arena *arena, scan_MatchList *matches);
b32             scan_view_eq(scan_ResultView lhs, scan_ResultView rhs);
