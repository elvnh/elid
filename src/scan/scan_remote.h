#pragma once

struct scan_Settings {
    /* memmi_MemoryRegionPermission mandatory_permissions; // TODO */
    /* b32   pause_while_scanning; */ // TODO
    usize required_alignment;
};

// TODO: move elsewhere
struct scan_Progress {
    usize *bytes_to_scan_ptr;
    usize *bytes_scanned_ptr;
};

// TODO: move elsewhere
struct scan_MultithreadArgs {
    WorkQueue *work_queue;
    usize max_bytes_to_scan_per_thread;
};

scan_Result scan_remote_memory_first(Arena **per_thread_result_arenas,
                                     memmi_Process proc,
                                     scan_Settings settings,
                                     scan_Comparison cmp,
                                     scan_Progress progress,
                                     scan_MultithreadArgs mt_args);

scan_Result scan_remote_memory_next(Arena **per_thread_result_arenas,
                                    memmi_Process proc,
                                    scan_Settings settings,
                                    scan_Comparison cmp,
                                    scan_Result prev_results,
                                    scan_Progress progress,
                                    scan_MultithreadArgs mt_args);

// TODO: move elsewhere
scan_Value_Maybe  scan_remote_read_single_value(memmi_Process proc, scan_ValueType value_type, uintptr_t address);

////////////////////////////////////////////////////////////////////////////////
// Single-threaded scans, reference implementation
////////////////////////////////////////////////////////////////////////////////
scan_Result scan_remote_memory_first_single_threaded(Arena *arena, memmi_Process proc, scan_Settings settings,
                                                     scan_Comparison cmp, scan_Progress progress);
scan_Result scan_remote_memory_next_single_threaded(Arena *arena, memmi_Process proc, scan_Settings settings,
                                                    scan_Comparison cmp, scan_Result prev_results, scan_Progress progress);
