#pragma once

struct app_UpdateAddressTableArgs {
    app_State *app;
    app_AddressTableEntry *entry;
};

struct app_UpdateResultTableArgs {
    app_State *app;
    scan_MatchDetails match_details;
    scan_ValueType value_type;
};

struct app_AsyncScanArgs {
    app_State *app;
    scan_Settings settings;
    scan_Comparison cmp;
};

void app_async_get_running_processes(TaskID, void *data);
void app_async_update_address_table_entry_value(TaskID, void *data);
void app_async_update_result_table_entry(TaskID, void *data);
void app_async_run_first_scan(TaskID, void *data);
void app_async_run_next_scan(TaskID, void *data);
