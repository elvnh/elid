#pragma once

// TODO: move elsewhere
enum gui_Popup {
    /* Normal popups */
    GUI_POPUP_OPEN_PROCESS,
    GUI_POPUP_EDIT_VALUE,
    GUI_POPUP_ADD_ADDRESS,

    /* Errors */
    GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
    GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
    GUI_POPUP_ERROR_WRITE_FAILED,

    GUI_POPUP_COUNT,
};

// TODO: because of all the textboxes, this struct is getting quite big, might be a good idea to
// allocate on heap
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
        scan_ValueType value_type;
    } edit_value_popup;

    struct {
        char address_textbox[256];
        scan_ValueType selected_value_type;
        app_AddressTableEntry *edited_entry;
    } edit_address_table_entry_popup;
};

b32 gui_init(gui_State *gui);
