#define SCAN_DEFAULT_STRIDE 1

struct scan_Match {
    scan_Value value;
    usize      offset_from_base;
};

// TODO: store in unrolled linked list for each thread, then main thread
// can either join those lists together or merge into flat array
// TODO: store exact scan results sparsely
struct scan_MatchList {
    scan_Match matches[2]; // TODO: come up with a good size for this
    usize match_count;
    scan_MatchList *next;
};

// TODO: instead return scan_MatchList * from scan_local_memory, let remote memory scanning function
// allocate a RegionMatches and fill out the details
struct scan_RegionMatches {
    // TODO: store type and mode?
    usize base_address;
    usize region_size;
    scan_MatchList *head;
};

struct scan_Result2 {
    // TODO: total match count?
    scan_RegionMatches *first;
    scan_RegionMatches *last;
};

scan_MatchList *scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, usize stride);
