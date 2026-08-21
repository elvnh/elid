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
    usize address;
};

// TODO: store exact scan results sparsely
struct scan_Result {
    // TODO: store type and mode?
    scan_Match *matches;
    usize match_count;
};

struct scan_Parameters {
    scan_Mode mode;
    scan_Type type;
    scan_Value args[3];
};

// 1-ary (exact, greater than)
// comparative (changed)
// parameterized comparative (changed by)

scan_Result scan_first(void *memory, usize memory_size, scan_Parameters params);
