#include "app_actions.h"

void app_action_get_running_processes(app_State *app)
{
    atomic_store_s32(&app->process_list_ready, false);
    wq_push_task(app->multi_frame_work_queue, app_async_get_running_processes, app);
}

b32 app_action_open_process(app_State *app, memmi_PID pid)
{
    b32 result = false;

    app_action_close_opened_process(app);

    memmi_OpenProcess open_process_result = memmi_open_process(pid, memmi_default_allocator());

    if (open_process_result.status == MEMMI_OK) {
        app->open_process_handle = open_process_result.process;
        result = true;
    }

    return result;
}

void app_action_close_opened_process(app_State *app)
{
    if (app->scanner_state != APP_SCANNER_STATE_FIRST_SCAN_READY) {
        app->scanner_state = APP_SCANNER_STATE_SWITCHED_PROCESS_AFTER_FIRST_SCAN;
    }

    if (!memmi_process_is_null(app->open_process_handle)) {
        memmi_close_process(app->open_process_handle, memmi_default_allocator());
    }

    app->open_process_handle = {};
}

void app_action__prepare_scan(app_State *app)
{
    app->scan_in_progress = true;
    app->bytes_to_scan = 0;
    app->bytes_scanned = 0;
}

void app_action_run_first_scan(app_State *app, scan_Settings settings, scan_Comparison cmp)
{
    ASSERT(!atomic_load_s32(&app->scan_in_progress));
    ASSERT((atomic_load_s32(&app->scanner_state) == APP_SCANNER_STATE_FIRST_SCAN_READY)
        || (atomic_load_s32(&app->scanner_state) == APP_SCANNER_STATE_SWITCHED_PROCESS_AFTER_FIRST_SCAN));

    app_action__prepare_scan(app);

    app_AsyncScanArgs args = {};
    args.app = app;
    args.settings = settings;
    args.cmp = cmp;

    wq_push_task(app->multi_frame_work_queue, app_async_run_first_scan, args);
}

void app_action_run_next_scan(app_State *app, scan_Settings settings, scan_Comparison cmp)
{
    ASSERT(!atomic_load_s32(&app->scan_in_progress));
    ASSERT(app->result_list.total_match_count > 0);
    ASSERT(atomic_load_s32(&app->scanner_state) == APP_SCANNER_STATE_NEXT_SCAN_READY);

    app_action__prepare_scan(app);

    app_AsyncScanArgs args = {};
    args.app = app;
    args.settings = settings;
    args.cmp = cmp;

    wq_push_task(app->multi_frame_work_queue, app_async_run_next_scan, args);
}

void app_action_reset_scan_results(app_State *app)
{
    ASSERT(app->scanner_state != APP_SCANNER_STATE_FIRST_SCAN_READY);

    app->scanner_state = APP_SCANNER_STATE_FIRST_SCAN_READY;
    app->result_list = {};

    app->bytes_scanned = 0;
    app->bytes_to_scan = 0;
}

void app_action_edit_value_at_address(app_State *app, uintptr_t address, scan_ValueType type, scan_Value new_value)
{
    // TODO: async

    usize size = scan_val_type_get_size(type);
    memmi_WriteMemory write_mem_result = memmi_write_memory(app->open_process_handle, address, &new_value, size);

    if (write_mem_result.status != MEMMI_OK) {
        app_push_error(app, {APP_ERROR_WRITE_FAILED});
    }
}

void app_action_update_address_table_entry_value(app_State *app, app_AddressTableEntry *entry)
{
    if (app_has_opened_process(app)) {
        app_UpdateAddressTableArgs arg = {};
        arg.app = app;
        arg.entry = entry;

        wq_push_task(app->per_frame_work_queue, app_async_update_address_table_entry_value, arg);
    }
}

void app_action_update_address_table_entry_synchronously(app_State *app, app_AddressTableEntry *entry)
{
    app_addr_table_entry_update_value(entry, app->open_process_handle);
}

void app_action_push_address_table_entry(app_State *app, app_AddressTableEntry *entry)
{
    // TODO: update immediately so that value is visible
    app_addr_table_push(&app->address_table, entry);
}

void app_action_update_result_table_entry(app_State *app, scan_MatchView match)
{
    app_UpdateResultTableArgs args = {};
    args.app = app;
    args.match_details = match;

    wq_push_task(app->per_frame_work_queue, app_async_update_result_table_entry, args);
}

void app_action_edit_address_table_entry(app_State *app, app_AddressTableEntry *entry, app_AddressTableEntry *new_entry)
{
    app_addr_table_change(&app->address_table, entry, new_entry);
}

void app_action_remove_address_table_entry(app_State *app, app_AddressTableEntry *entry)
{
    app_addr_table_remove(&app->address_table, entry);
}

static void *app_action__watch_entry_point(void *data)
{
    app_Watch *watch = (app_Watch *)data;

    app_watch_run(watch);

    return 0;
}

void app_action_begin_watch(app_State *app, uintptr_t address, app_WatchCondition cond, u32 length)
{
    ASSERT(!memmi_process_is_null(app->open_process_handle));
    ASSERT(!app->watch);

    app->watch = pool_alloc(app->watch_pool, app_Watch);

    // NOTE: for now, we always use breakpoint index 0
    app_watch_init(app->watch, app->open_process_handle, address, 0, cond, length);

    app->watch->thread = thread_create(app_action__watch_entry_point, app->watch);
}

void app_action_end_watch(app_State *app)
{
    if (ALWAYS(app->watch)) {
        app_watch_destroy(app->watch_pool, app->watch);

        app->watch = 0;
    }
}

void app_action_poll_watch_events(app_State *app)
{
    if (app->watch) {
        proc_Status poll_result = app_watch_poll_events(app->watch);
        // TODO: why is this assertion triggered?
        // ASSERT(poll_result == PROC_STATUS_OK);
    }
}
