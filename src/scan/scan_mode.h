#pragma once

enum scan_Mode {
    // Non-comparative scan modes, i.e doesn't rely on previous value
    SCAN_MODE_EXACT,
    SCAN_MODE_UNKNOWN,
    SCAN_MODE_GREATER_THAN,
    SCAN_MODE_LESS_THAN,

    // Comparative scan modes, relies on previous value
    SCAN_MODE_INCREASED,
    SCAN_MODE_INCREASED_BY,
    SCAN_MODE_DECREASED,
    SCAN_MODE_DECREASED_BY,
    SCAN_MODE_CHANGED,
    SCAN_MODE_CHANGED_BY,
    SCAN_MODE_UNCHANGED,

    SCAN_MODE_COUNT
};

struct scan_ModeList {
    scan_Mode data[SCAN_MODE_COUNT];
    usize     count;
};

b32           scan_mode_is_comparative(scan_Mode mode);
const char   *scan_mode_to_cstr(scan_Mode mode);
scan_ModeList scan_mode_get_list();
u32           scan_mode_get_arity(scan_Mode mode);
