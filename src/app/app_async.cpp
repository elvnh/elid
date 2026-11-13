#include "app_async.h"

// TODO: come up with a good default and make it customizable
#define APP_DEFAULT_MAX_BYTES_TO_SCAN_PER_THREAD MB(1)

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

    scan_Value value = {};
    // TODO: just call the function in app_address_table.h
    if (app_read_single_value_in_process(app->open_process_handle, entry->address, entry->value_type, &value)) {
        scan_val_atomic_store(&entry->current_value, value);
        atomic_store_s32(&entry->last_read_successful, true);
    } else {
        atomic_store_s32(&entry->last_read_successful, false);
    }
}

void app_async_update_result_table_entry(TaskID, void *data)
{
    app_UpdateResultTableArgs *args = (app_UpdateResultTableArgs *)data;

    scan_MatchView details = args->match_details;

    uintptr_t address = details.match->address;

    scan_Value value = {};

    // TODO: display null value if failed to read
    // TODO: don't do this every frame

    if (app_read_single_value_in_process(args->app->open_process_handle, address, args->value_type, &value)) {
        scan_Value new_curr_value = value;
        scan_Value prev_curr_value = scan_val_atomic_load(&details.match->value);

        b32 value_has_changed = !scan_val_eq(args->value_type, prev_curr_value, new_curr_value);

        if (value_has_changed) {
            scan_val_atomic_store(&details.history->previous_value, prev_curr_value);
            scan_val_atomic_move(&details.match->value, &value);

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

static void app__reset_current_per_thread_scan_result_arenas(app_State *app)
{
    for (usize i = 0; i < wq_get_thread_count(app->multi_frame_work_queue); ++i) {
        arena_reset(app->current_per_thread_scan_result_arenas[i]);
    }
}

void app_async_run_first_scan(TaskID, void *data)
{
    app_AsyncScanArgs args = *(app_AsyncScanArgs *)data;
    app_State *app = args.app;

    scan_Progress progress = {};
    progress.bytes_to_scan_ptr = &app->bytes_to_scan;
    progress.bytes_scanned_ptr = &app->bytes_scanned;

    app__reset_current_per_thread_scan_result_arenas(app);

    #if defined(SINGLE_THREADED_SCANS)
    {
        app->result_list = scan_remote_memory_first(
            app->current_per_thread_scan_result_arenas[0], app->open_process_handle, args.settings, args.cmp, progress);
    }
    #else
    {
        scan_MultithreadArgs mt_args = {};
        mt_args.work_queue = app->multi_frame_work_queue;
        mt_args.max_bytes_to_scan_per_thread = APP_DEFAULT_MAX_BYTES_TO_SCAN_PER_THREAD;

        app->result_list = scan_remote_memory_first(
            app->current_per_thread_scan_result_arenas, app->open_process_handle, args.settings, args.cmp, progress, mt_args);

        #if DEBUG_BUILD
        {
            scan_Progress prog = {};
            usize dummy = 0;
            prog.bytes_to_scan_ptr = &dummy;
            prog.bytes_scanned_ptr = &dummy;

            scan_Result result_old = scan_remote_memory_first_single_threaded(
                app->current_per_thread_scan_result_arenas[0], app->open_process_handle,
                args.settings, args.cmp, prog);

            ScratchArena scratch = scratch_begin();
            scan_ResultView view_new = scan_result_view_from_result(scratch.arena, app->result_list);
            scan_ResultView view_old = scan_result_view_from_result(scratch.arena, result_old);

            ASSERT(scan_result_view_eq(view_new, view_old));
        }
        #endif
    }
    #endif

    app_async__maybe_push_scan_error(app, app->result_list.errors);

    atomic_store_s32(&app->scanner_state, APP_SCANNER_STATE_NEXT_SCAN_READY);
    atomic_store_s32(&app->scan_in_progress, false);
}

void app_async_run_next_scan(TaskID, void *data)
{
    app_AsyncScanArgs args = *(app_AsyncScanArgs *)data;
    app_State *app = args.app;

    scan_Progress progress = {};
    progress.bytes_to_scan_ptr = &app->bytes_to_scan;
    progress.bytes_scanned_ptr = &app->bytes_scanned;

    SWAP(app->current_per_thread_scan_result_arenas, app->previous_per_thread_scan_result_arenas);

    app__reset_current_per_thread_scan_result_arenas(app);

    #if defined(SINGLE_THREADED_SCANS)
    {
        app->result_list = scan_remote_memory_next(
            app->current_per_thread_scan_result_arenas[0],
            app->open_process_handle, args.settings, args.cmp, app->result_list, progress);
    }
    #else
    {
        scan_MultithreadArgs mt_args = {};
        mt_args.work_queue = app->multi_frame_work_queue;
        mt_args.max_bytes_to_scan_per_thread = APP_DEFAULT_MAX_BYTES_TO_SCAN_PER_THREAD;

        scan_Result previous_results = app->result_list;
        app->result_list = scan_remote_memory_next(
            app->current_per_thread_scan_result_arenas,
            app->open_process_handle, args.settings, args.cmp, previous_results, progress, mt_args);

        #if DEBUG_BUILD
        {
            scan_Progress prog = {};
            usize dummy = 0;
            prog.bytes_to_scan_ptr = &dummy;
            prog.bytes_scanned_ptr = &dummy;

            scan_Result result_old = scan_remote_memory_next_single_threaded(
                app->current_per_thread_scan_result_arenas[0],
                app->open_process_handle, args.settings, args.cmp, previous_results, prog);

            ScratchArena scratch = scratch_begin();
            scan_ResultView view_new = scan_result_view_from_result(scratch.arena, app->result_list);
            scan_ResultView view_old = scan_result_view_from_result(scratch.arena, result_old);

            ASSERT(scan_result_view_eq(view_new, view_old));
        }
        #endif
    }
    #endif

    app_async__maybe_push_scan_error(app, app->result_list.errors);

    atomic_store_s32(&args.app->scan_in_progress, false);
}
