#pragma once

void gui_popup_running_processes(gui_State *gui);
// TODO: don't pass app to these
void gui_popup_edit_value(app_State *app, gui_State *gui, scan_ValueType type, uintptr_t address, scan_Value_Maybe curr_value);
void gui_popup_edit_address_table_entry(app_State *app, gui_State *gui, app_AddressTableEntry *optional_entry);
void gui_popup_error(gui_State *gui, app_Error error);
void gui_popup_display_active_popups(gui_State *gui, app_State *app);
