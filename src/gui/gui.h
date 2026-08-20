#pragma once

// TODO: rename to gui_State
struct Gui {
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
        s32 selected_scan_mode_index;
        s32 selected_value_type_index;
        bool fast_scan;
        bool pause_while_scanning;
    } scan_parameters;
};

b32             gui_init(Gui *gui);
b32             gui_should_close(Gui *gui);
app_CommandList gui_update_and_render(Gui *gui, app_State *app);
