#pragma once

scan_Result       scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Comparison cmp, usize stride);
scan_Result       scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Comparison cmp, scan_Result prev_results);
