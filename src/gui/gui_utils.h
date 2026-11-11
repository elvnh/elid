#pragma once

DEFINE_MAYBE_TYPE(scan_Comparison);

scan_Value_Maybe       gui_try_parse_search_value(const char *input, scan_ValueType type, u32 base);

// TODO: rename
scan_Comparison_Maybe  gui_try_parse_scan_input(const char *input, scan_ValueType type, u32 base, scan_Mode mode);
usize_Maybe            gui_try_parse_address(const char *input, u32 base);
char                  *gui_cstr_from_address(Arena *arena, uintptr_t address);
char                  *gui_cstr_from_match_address(Arena *arena, scan_MatchView match_details, proc_ObjectList objects);
// TODO: rename
char                  *gui_cstr_from_match_value(Arena *arena, scan_Value value, scan_ValueType value_type);
char                  *gui_cstr_from_value_type(Arena *arena, scan_ValueType type);
scan_Settings          gui_get_scan_settings(gui_State *gui);
void                   gui_push_no_selectable_highlight(void);
void                   gui_pop_no_selectable_highlight(void);
void                   gui_set_initial_keyboard_focus_here(s32 offset = 0);

// TODO: move this elsewhere
scan_ValueType gui_value_type_dropdown(gui_State *gui, const char *title, scan_ValueType current_type, f32 width);
