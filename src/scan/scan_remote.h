#pragma once

struct scan_Settings {
    /* memmi_MemoryRegionPermission mandatory_permissions; // TODO */
    /* b32   pause_while_scanning; */ // TODO
    usize required_alignment;
};

struct scan_Progress {
    usize *bytes_to_scan_ptr;
    usize *bytes_scanned_ptr;
};

scan_Result scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp,
                                    scan_Progress progress);
scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp,
                                    scan_Result prev_results, scan_Progress progress);

// TODO: move elsewhere
scan_Value_Maybe  scan_remote_read_single_value(memmi_Process proc, scan_ValueType value_type, uintptr_t address);
