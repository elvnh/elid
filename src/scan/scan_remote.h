#pragma once

struct scan_Settings {
    /* memmi_MemoryRegionPermission mandatory_permissions; // TODO */
    /* b32   pause_while_scanning; */ // TODO
    usize required_alignment;
};

scan_Result scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp);
scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp, scan_Result prev_results);
