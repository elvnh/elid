#define SCAN_DEFAULT_STRIDE 1

struct scan_Match {
    scan_Value value;
    usize      offset_from_base;
};

// TODO: store in unrolled linked list for each thread, then main thread
// can either join those lists together or merge into flat array
// TODO: store exact scan results sparsely
struct scan_MatchList {
    scan_Match data[2]; // TODO: come up with a good size for this
    usize count;
    scan_MatchList *next;
};

struct scan_RegionMatches {
    usize base_address;
    usize region_size;
    scan_MatchList *head;
};

struct scan_Result2 {
    // TODO: store type and mode?
    // TODO: total match count?
    scan_RegionMatches *first;
    scan_RegionMatches *last;
};

scan_MatchList *scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, usize stride);
scan_MatchList *scan_local_memory_next(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, scan_MatchList *prev_matches);
