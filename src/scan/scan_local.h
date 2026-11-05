#pragma once

struct scan_MultithreadArgs {
    WorkQueue *work_queue;
    usize max_bytes_to_scan_per_thread;
    TaskID parent_task;
};

void scan_local_memory_first_new(Arena **per_thread_result_arenas, scan_MatchList *per_thread_match_lists,
    void *mem, usize mem_size, uintptr_t base_address, scan_Comparison cmp, usize alignment, scan_MultithreadArgs mt_args,
    usize local_offset /* TODO: remove */);

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, uintptr_t base_address,
                                        scan_Comparison cmp, usize alignment);
scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp,
                                       scan_MatchList *prev_matches);
