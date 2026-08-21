// TODO: better name for this file

enum scan_Mode {
    // Non-comparative scan modes
    SCAN_MODE_EXACT,
    SCAN_MODE_GREATER_THAN,

    // Comparative scan modes
    SCAN_MODE_CHANGED,
    SCAN_MODE_CHANGED_BY,
};

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

// TODO: helper functions for creating these for types and modes
struct scan_Parameters {
    scan_Mode mode;
    scan_Type type;
    scan_Value args[3];
};

scan_Result scan_local_memory_first(Arena *arena, void *memory, usize memory_size, scan_Parameters params, uintptr_t base_address);
b32         scan_mode_is_comparative(scan_Mode mode);
