#pragma once

// TODO: rename to gui_State
struct gui_State {
    GLFWwindow *window;

    struct {
        s32 selected_index;
    } process_list_popup;

    struct {
        s32 selected_row_index;
    } result_table;

    struct {
        char scan_value_text[256];
        bool hex;
        scan_Mode selected_scan_mode;
        scan_ValueType selected_value_type;
        bool fast_scan;
        bool pause_while_scanning;
    } scan_parameters;
};

b32 gui_init(gui_State *gui);
