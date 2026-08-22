#define SCAN_DEFAULT_STRIDE 1

// TODO: better name for this file
struct scan_Match {
    scan_Value value;
    usize remote_address;
};

// TODO: store in unrolled linked list for each thread, then main thread
// can either join those lists together or merge into flat array
// TODO: store exact scan results sparsely
struct scan_Result {
    // TODO: store type and mode?
    scan_Match *matches;
    usize match_count;
};

struct scan_Stride {
    usize stride;
};

scan_Result scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Comparison cmp, uintptr_t base_address, usize stride);
