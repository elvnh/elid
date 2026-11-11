#pragma once

// This file defines all actions the application can take. The GUI should only call
// functions defined here.
void app_action_get_running_processes(app_State *app);
b32  app_action_open_process(app_State *app, memmi_PID pid);
void app_action_close_opened_process(app_State *app);
void app_action_run_first_scan(app_State *app, scan_Settings settings, scan_Comparison cmp);
void app_action_run_next_scan(app_State *app, scan_Settings settings, scan_Comparison cmp);
void app_action_reset_scan_results(app_State *app);
void app_action_edit_value_at_address(app_State *app, uintptr_t address, scan_ValueType type, scan_Value new_value);
void app_action_push_address_table_entry(app_State *app, app_AddressTableEntry entry);
void app_action_update_address_table_entry_value(app_State *app, app_AddressTableEntry *entry);
void app_action_edit_address_table_entry(app_State *app, app_AddressTableEntry *entry, app_AddressTableEntry new_entry);
void app_action_remove_address_table_entry(app_State *app, app_AddressTableEntry *entry);
void app_action_update_result_table_entry(app_State *app, scan_MatchView match);
void app_action_begin_watch(app_State *app, uintptr_t address, app_WatchCondition cond, u32 length);
void app_action_end_watch(app_State *app);
void app_action_poll_watch_events(app_State *app);
