#pragma once

// TODO: move elsewhere
enum gui_Popup {
    /* Normal popups */
    GUI_POPUP_OPEN_PROCESS,
    GUI_POPUP_EDIT_VALUE,

    /* Errors */
    GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
    GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,

    GUI_POPUP_COUNT,
};

struct gui_State {
    GLFWwindow *window;

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

    b32 popup_states[GUI_POPUP_COUNT];

    struct {
        s32 selected_index;
    } process_list_popup;

    struct {
        char value_textbox[256];
        uintptr_t edited_address;
    } edit_value_popup;
};

b32 gui_init(gui_State *gui);
