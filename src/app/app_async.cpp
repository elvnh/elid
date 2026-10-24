#include "app_async.h"

void app_async_get_running_processes(app_TaskID, void *data)
{
    app_State *app = *(app_State **)data;

    arena_reset(app->process_list_arena);
    memmi_Allocator allocator = proc_memmi_allocator_from_arena(app->process_list_arena);

    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;

    atomic_store_s32(&app->process_list_ready, true);
}

void app_async_update_address_table_entry_value(app_TaskID, void *data)
{
    app_UpdateAddressTableArgs *args = (app_UpdateAddressTableArgs *)data;
    app_AddressTableEntry *entry = args->entry;
    app_State *app = args->app;

    scan_Value_Maybe value_opt = scan_remote_read_single_value(app->open_process_handle, entry->value_type, entry->address);

    if (value_opt.ok) {
        scan_val_atomic_store(&entry->current_value, value_opt.value);
        atomic_store_s32(&entry->last_read_successful, true);
    } else {
        atomic_store_s32(&entry->last_read_successful, false);
    }
}

void app_async_update_result_table_entry(app_TaskID, void *data)
{
    app_UpdateResultTableArgs *args = (app_UpdateResultTableArgs *)data;

    scan_MatchDetails details = args->match_details;

    uintptr_t address = scan_address_from_match_details(details);
    scan_Value_Maybe value_opt = scan_remote_read_single_value(args->app->open_process_handle, args->value_type, address);

    // TODO: display null value if failed to read
    // TODO: don't do this every frame

    if (value_opt.ok) {
        scan_Value new_curr_value = value_opt.value;
        scan_Value prev_curr_value = scan_val_atomic_load(&details.match->value);

        b32 value_has_changed = !scan_val_eq(args->value_type, prev_curr_value, new_curr_value);

        if (value_has_changed) {
            scan_val_atomic_store(&details.history->previous_value, prev_curr_value);
            scan_val_atomic_move(&details.match->value, &value_opt.value);

            printf("%p: %u -> %u\n",
                (void *)address,
                atomic_load_u32(&details.history->previous_value.int32),
                atomic_load_u32(&details.match->value.int32));
        }
    }
}

void app_async_watch_address(app_TaskID, void *data)
{
    app_Watch *watch = *(app_Watch **)data;

    app_watch_run(watch);
}
