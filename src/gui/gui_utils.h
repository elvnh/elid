#pragma once

DEFINE_MAYBE_TYPE(scan_Value);
DEFINE_MAYBE_TYPE(scan_Comparison);

scan_Value_Maybe       gui_try_parse_search_value(const char *input, scan_ValueType type, u32 base);

// TODO: rename
scan_Comparison_Maybe  gui_try_parse_scan_input(const char *input, scan_ValueType type, u32 base, scan_Mode mode);
char                  *gui_cstr_from_match_address(Arena *arena, scan_MatchDetails match_details, proc_ObjectList objects);
char                  *gui_cstr_from_match_value(Arena *arena, scan_Value value, scan_ValueType value_type);
