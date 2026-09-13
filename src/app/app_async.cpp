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

    Temp temp = scratch_begin();
    memmi_Allocator allocator = proc_memmi_allocator_from_arena(temp.arena);

    // It's fine to read these members as long as we do it atomically, as even if the entry gets
    // deleted the memory will still be valid since the nodes are recycled.
    scan_ValueType value_type = atomic_load_s32(&entry->value_type);
    uintptr_t address = atomic_load_usize(&entry->address);

    usize size = scan_val_type_get_size(value_type);
    memmi_ReadMemory read_mem_result = memmi_read_memory(app->open_process_handle, address, size, allocator);

    if (read_mem_result.status == MEMMI_OK) {
        scan_Value value = {};
        copy_mem(&value, read_mem_result.memory, 1, read_mem_result.bytes_read);

        scan_val_atomic_store(&entry->current_value, value);
        atomic_store_s32(&entry->last_read_successful, true);
    } else {
        atomic_store_s32(&entry->last_read_successful, false);
    }
}
