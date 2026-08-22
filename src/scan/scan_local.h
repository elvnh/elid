#define SCAN_DEFAULT_STRIDE 1

struct scan_Match {
    scan_Value value;
    usize      offset_from_base;
};

// TODO: store in unrolled linked list for each thread, then main thread
// can either join those lists together or merge into flat array
// TODO: store exact scan results sparsely
// TODO: don't push to front of list, we probably want the results in order of increasing address
struct scan_MatchList {
    scan_Match data[2]; // TODO: come up with a good size for this
    usize count;
    scan_MatchList *next;
};

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, usize stride);
scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp, scan_MatchList *prev_matches);
