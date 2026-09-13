#pragma once

struct app_UpdateAddressTableArgs {
    app_State *app;
    app_AddressTableEntry *entry;
};

void app_async_get_running_processes(app_TaskID, void *data);
void app_async_update_address_table_entry_value(app_TaskID, void *data);
