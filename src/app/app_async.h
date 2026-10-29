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

void app_async_get_running_processes(app_TaskID, void *data);
void app_async_update_address_table_entry_value(app_TaskID, void *data);
void app_async_update_result_table_entry(app_TaskID, void *data);
void app_async_run_first_scan(app_TaskID, void *data);
