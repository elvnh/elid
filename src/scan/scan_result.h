#pragma once

// TODO: come up with a good size for this
#define SCAN_MATCH_LIST_SIZE 2

struct scan_Match {
    scan_Value value;
    usize      offset_from_base;
};

struct scan_MatchHistory {
    scan_Value previous_value;
    scan_Value first_value;
};

// TODO: store exact scan results sparsely
struct scan_MatchList {
    scan_Match        match_data[SCAN_MATCH_LIST_SIZE];
    scan_MatchHistory match_history_data[SCAN_MATCH_LIST_SIZE];
    usize count; // Number of matches in this list.

    usize total_match_count; // Total number of matches in chain.

    // TODO: pack list node members into struct
    scan_MatchList *next;
    scan_MatchList *first;
    scan_MatchList *last;
    usize list_node_count;
};

struct scan_RegionMatches {
    // TODO: just store memmi region here?
    usize  region_base_address;
    usize  region_size;

    proc_Object_Maybe containing_object;

    usize match_count_in_region;

    scan_MatchList *head;
    scan_RegionMatches *next;
};

struct scan_Result {
    scan_Mode           scan_mode;
    scan_ValueType      value_type;
    proc_Status         errors;
    usize               total_match_count;

    proc_ObjectList     process_objects;

    scan_RegionMatches *first;
    scan_RegionMatches *last;
    usize               list_node_count;
};

struct scan_MatchDetails {
    uintptr_t   address;
    scan_Value  value;
    proc_Object_Maybe containing_object;
};

scan_MatchDetails scan_result_at(scan_Result *matches, usize at);
