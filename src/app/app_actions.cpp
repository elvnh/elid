#include "app_actions.h"

void app_action_get_running_processes(app_State *app)
{
    arena_reset(app->process_list_arena);

    memmi_Allocator allocator = proc_memmi_allocator_from_arena(app->process_list_arena);

    // TODO: do this async, and display empty window in meantime?
    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;
}

b32 app_action_open_process(app_State *app, memmi_PID pid)
{
    b32 result = false;

    app_action_close_opened_process(app);

    memmi_OpenProcess open_process_result = memmi_open_process(pid);

    if (open_process_result.status == MEMMI_OK) {
        app->open_process_handle = open_process_result.process;
        result = true;
    }

    return result;

}

void app_action_close_opened_process(app_State *app)
{
    memmi_close_process(app->open_process_handle);
    app->open_process_handle = {};
}

void app_action_run_first_scan(app_State *app, scan_Settings settings, scan_Comparison cmp)
{
    // TODO: async
    arena_reset(app->scan_result_arena);
    arena_reset(app->previous_scan_result_arena);

    app->result_list = scan_remote_memory_first(
        app->scan_result_arena, app->open_process_handle, settings, cmp);

    proc_Status errors = app->result_list.errors;

    if (errors != PROC_STATUS_OK) {
        if (errors & PROC_STATUS_NO_SUCH_PROCESS) {
            app_push_error(app, {APP_ERROR_PROCESS_DIED_BEFORE_OR_DURING_SCAN});
        } else if (errors & PROC_STATUS_INSUFFICIENT_PERMISSIONS) {
            app_push_error(app, {APP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN});
        }
    }

    app->scanner_state = APP_SCANNER_STATE_NEXT_SCAN_READY;
}

void app_action_run_next_scan(app_State *app, scan_Settings settings, scan_Comparison cmp)
{
    // TODO: async
    ASSERT(app->result_list.total_match_count > 0);
    ASSERT(app->scanner_state == APP_SCANNER_STATE_NEXT_SCAN_READY);

    SWAP(app->scan_result_arena, app->previous_scan_result_arena);

    arena_reset(app->scan_result_arena);

    app->result_list = scan_remote_memory_next(
        app->scan_result_arena, app->open_process_handle, settings, cmp, app->result_list);
}

void app_action_reset_scan_results(app_State *app)
{
    ASSERT(app->scanner_state != APP_SCANNER_STATE_FIRST_SCAN_READY);

    arena_reset(app->scan_result_arena);
    arena_reset(app->previous_scan_result_arena);

    app->scanner_state = APP_SCANNER_STATE_FIRST_SCAN_READY;
    app->result_list = {};

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
    // TODO: async
    // TODO: make it possible to have arena use stack buffer

    if (app_has_opened_process(app)) {
        Temp temp = scratch_begin();
        memmi_Allocator allocator = proc_memmi_allocator_from_arena(temp.arena);

        usize size = scan_val_type_get_size(entry->value_type);

        memmi_ReadMemory read_mem_result = memmi_read_memory(app->open_process_handle, entry->address, size, allocator);

        if (read_mem_result.status == MEMMI_OK) {
            copy_mem(&entry->current_value, read_mem_result.memory, 1, read_mem_result.bytes_read);
            entry->last_read_successful = true;
        } else {
            entry->last_read_successful = false;
        }
    }
}

void app_action_push_address_table_entry(app_State *app, app_AddressTableEntry entry)
{
    // TODO: update immediately
    app_addr_table_push(&app->address_table, entry);
}
