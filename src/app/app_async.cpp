#include "app_async.h"

void app_async_get_running_processes(TaskID, void *data)
{
    app_State *app = *(app_State **)data;

    arena_reset(app->process_list_arena);
    memmi_Allocator allocator = proc_memmi_allocator_from_arena(app->process_list_arena);

    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;

    atomic_store_s32(&app->process_list_ready, true);
}

void app_async_update_address_table_entry_value(TaskID, void *data)
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

void app_async_update_result_table_entry(TaskID, void *data)
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

static void app_async__maybe_push_scan_error(app_State *app, proc_Status status)
{
    if (status != PROC_STATUS_OK) {
        if (status & PROC_STATUS_NO_SUCH_PROCESS) {
            app_push_error(app, {APP_ERROR_PROCESS_DIED_BEFORE_OR_DURING_SCAN});
        } else if (status & PROC_STATUS_INSUFFICIENT_PERMISSIONS) {
            app_push_error(app, {APP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN});
        }
    }
}

void app_async_run_first_scan(TaskID, void *data)
{
    app_AsyncScanArgs args = *(app_AsyncScanArgs *)data;
    app_State *app = args.app;

    scan_Progress progress = {};
    progress.bytes_to_scan_ptr = &app->bytes_to_scan;
    progress.bytes_scanned_ptr = &app->bytes_scanned;

    arena_reset(app->scan_result_arena);
    arena_reset(app->previous_scan_result_arena);

    // TODO: multithread scans
    app->result_list = scan_remote_memory_first(
        app->scan_result_arena, app->open_process_handle, args.settings, args.cmp, progress);

    atomic_store_s32(&app->scan_in_progress, false);
    atomic_store_s32(&app->scanner_state, APP_SCANNER_STATE_NEXT_SCAN_READY);

    app_async__maybe_push_scan_error(app, app->result_list.errors);
}

void app_async_run_next_scan(TaskID, void *data)
{
    app_AsyncScanArgs args = *(app_AsyncScanArgs *)data;
    app_State *app = args.app;

    scan_Progress progress = {};
    progress.bytes_to_scan_ptr = &app->bytes_to_scan;
    progress.bytes_scanned_ptr = &app->bytes_scanned;

    SWAP(app->scan_result_arena, app->previous_scan_result_arena);

    arena_reset(app->scan_result_arena);

    app->result_list = scan_remote_memory_next(
        app->scan_result_arena, app->open_process_handle, args.settings, args.cmp, app->result_list, progress);

    app_async__maybe_push_scan_error(app, app->result_list.errors);

    atomic_store_s32(&args.app->scan_in_progress, false);
}
