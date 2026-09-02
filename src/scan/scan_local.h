#pragma once

#define SCAN_DEFAULT_STRIDE 1

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, usize stride);
scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, scan_MatchList *prev_matches);
