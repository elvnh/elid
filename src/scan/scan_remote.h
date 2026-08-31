#pragma once

// TODO: simplify the result structs, one layer of indirection can probably be removed

struct scan_RegionMatches {
    // TODO: just store memmi region here?
    usize region_base_address;
    usize region_size;
    b32   region_is_at_static_offset;

    usize match_count_in_region;

    scan_MatchList *head;
    scan_RegionMatches *next;
};

struct scan_Result {
    scan_Mode           scan_mode;
    scan_ValueType      value_type;
    proc_Status         errors;
    usize               total_match_count;

    scan_RegionMatches *first;
    scan_RegionMatches *last;
    usize               list_node_count;
};

struct scan_MatchDetails {
    uintptr_t  address;
    scan_Value value;
    b32        is_at_static_offset;
};

scan_Result       scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Comparison cmp, usize stride);
scan_Result       scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Comparison cmp, scan_Result prev_results);
scan_MatchDetails scan_result_at(scan_Result *matches, usize at);
