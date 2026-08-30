#pragma once

struct scan_RegionMatches {
    // TODO: just store memmi region here?
    usize base_address;
    usize region_size;
    scan_MatchList *head;
    scan_RegionMatches *next;
    usize match_count_in_region;
};

struct scan_Result {
    // TODO: store type and mode?
    proc_Status         errors;
    usize               total_match_count;
    scan_RegionMatches *first;
    scan_RegionMatches *last;
    usize               list_node_count;
};

struct scan_MatchDetails {
    uintptr_t  address;
    scan_Value value;
};

scan_Result       scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Comparison cmp, usize stride);
scan_Result       scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Comparison cmp, scan_Result prev_results);
scan_MatchDetails scan_result_at(scan_Result *matches, usize at);
