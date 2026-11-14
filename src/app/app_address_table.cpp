#include "app_address_table.h"

#define APP_ADDR_TABLE_INIT_NODE_CAP 128

b32 app_addr_table_init(Arena *arena, app_AddressTable *tbl)
{
    b32 result = false;

    *tbl = {};

    tbl->pool = pool_create_untyped(
        arena,
        MAX(sizeof(app_AddressTableEntry), sizeof(app_MemoryLocation)),
        APP_ADDR_TABLE_INIT_NODE_CAP,
        MAX(alignof(app_AddressTableEntry), alignof(app_MemoryLocation)));

    result = tbl->pool != 0;

    return result;
}

void app_addr_table__delete_entry(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    if (entry) {
        app_MemoryLocation *offs = list_head(&entry->pointer_offsets);

        while (offs) {
            app_MemoryLocation *next = offs->next;

            pool_free(tbl->pool, offs);

            offs = next;
        }

        dll_remove(tbl, entry);

        pool_free(tbl->pool, entry);
    }
}
void app_addr_table_delete_flagged_entries(app_AddressTable *tbl)
{
    app_AddressTableEntry *e = list_head(tbl);

    while (e) {
        app_AddressTableEntry *next = e->next;

        if (e->flagged_for_deletion) {
            app_addr_table__delete_entry(tbl, e);
        }

        e = next;
    }
}

app_AddressTableEntry *app_addr_table_entry_alloc(Pool *pool)
{
    app_AddressTableEntry *result = pool_alloc(pool, app_AddressTableEntry);
    result->allocated_in_pool = pool;

    return result;
}

void app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    dll_push_back(tbl, entry);
}

void app_addr_table_remove(app_AddressTableEntry *entry)
{
    entry->flagged_for_deletion = true;
}

void app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry *new_entry)
{
    dll_insert_after(tbl, new_entry, entry);
    dll_remove(tbl, entry);

    // Deleting the entry directly here is fine, since it's no longer part of the linked list and
    // won't be found when iterating.
    app_addr_table__delete_entry(tbl, entry);
}

usize app_addr_table_count(app_AddressTable *tbl)
{
    usize result = tbl->list_node_count;

    return result;
}

app_AddressTableEntry *app_addr_table_entry_clone(Pool *pool, app_AddressTableEntry *entry)
{
    app_AddressTableEntry *result = pool_alloc(pool, app_AddressTableEntry);
    *result = *entry;

    result->allocated_in_pool = pool;

    result->next = 0;
    result->prev = 0;

    result->pointer_offsets = {};

    for (app_MemoryLocation *offs = list_head(&entry->pointer_offsets); offs; offs = list_next(offs)) {
        app_MemoryLocation *offs_copy = pool_alloc(pool, app_MemoryLocation);
        *offs_copy = *offs;

        dll_push_back(&result->pointer_offsets, offs_copy);
    }

    return result;
}

app_MemoryLocation *app_addr_table_entry_push_offset(app_AddressTableEntry *entry, app_MemoryLocation *before)
{
    app_MemoryLocation *result = pool_alloc(entry->allocated_in_pool, app_MemoryLocation);

    if (before) {
        dll_insert_before(&entry->pointer_offsets, result, before);
    } else {
        dll_push_back(&entry->pointer_offsets, result);
    }

    app_addr_table_entry_initialize_offset_textbox(result);

    return result;
}

void app_addr_table_entry_remove_offset(app_AddressTableEntry *entry, app_MemoryLocation *offs)
{
    dll_remove(&entry->pointer_offsets, offs);
    pool_free(entry->allocated_in_pool, offs);
}

b32 app_addr_table_entry_swap_offsets(app_MemoryLocation *a, app_MemoryLocation *b)
{
    b32 result = false;

    if (a && b) {
        result = true;

        app_MemoryLocation *a_prev = a->prev;
        app_MemoryLocation *a_next = a->next;

        app_MemoryLocation *b_prev = b->prev;
        app_MemoryLocation *b_next = b->next;

        app_MemoryLocation temp = *a;
        *a = *b;
        *b = temp;

        a->prev = a_prev;
        a->next = a_next;

        b->prev = b_prev;
        b->next = b_next;
    }

    return result;
}

uintptr_t app_addr_table_entry_get_address(app_AddressTableEntry *entry)
{
    uintptr_t result = entry->base_address.last_resolved_address;

    return result;
}

void app_addr_table_entry_set_address(app_AddressTableEntry *entry, uintptr_t value)
{
    atomic_store_ptr(&entry->base_address.last_resolved_address, value);
}

b32 app_addr_table_entry_try_get_value(app_AddressTableEntry *entry, scan_Value *out)
{
    b32 result = atomic_load_s32(&entry->base_address.last_read_successful);

    if (result) {
        scan_val_atomic_move(out, &entry->base_address.last_dereferenced_value);
    }

    return result;
}

b32 app_addr_table_entry_try_get_final_value(app_AddressTableEntry *entry, scan_Value *out)
{
    b32 result = false;

    if (entry->kind == APP_ADDR_TABLE_ENTRY_NORMAL) {
        result = entry->base_address.last_read_successful;

        if (result) {
            *out = entry->base_address.last_dereferenced_value;

        }
    } else {
        ASSERT(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER);
        result = entry->final_pointer_offset.last_read_successful;

        if (result) {
            *out = entry->final_pointer_offset.last_dereferenced_value;
        }
    }

    return result;

}

b32 app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out)
{
    b32 result = false;

    if (entry->kind == APP_ADDR_TABLE_ENTRY_NORMAL) {
        // The first address is always considered valid.
        result = true;

        *out = entry->base_address.last_resolved_address;
    } else {
        // For pointer chains, the address of the current node is dependent on the previous
        // node. Therefore, the address is considered valid as long as the read of the previous
        // node succeeded, even if the read of this node failed.
        ASSERT(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER);

        app_MemoryLocation *prev_offs = entry->pointer_offsets.last;

        if (prev_offs) {
            result = prev_offs->last_read_successful;

            if (result) {
                *out = entry->final_pointer_offset.last_resolved_address;
            }
        } else {
            // We can't have failed to read the first address.
            *out = entry->base_address.last_resolved_address;
        }
    }

    return result;
}

static b32 app_addr_table__update_memory_location(
    app_MemoryLocation *loc, memmi_Process proc, uintptr_t *current_address, scan_ValueType type)
{
    b32 result = false;

    loc->last_resolved_address = *current_address;

    result = app_read_single_value_in_process(
        proc,
        *current_address + loc->address_offset,
        type,
        &loc->last_dereferenced_value);

    loc->last_read_successful = result;

    *current_address = loc->last_dereferenced_value.pointer;

    return result;
}

void app_addr_table_offset_propagate_success_status(app_AddressTableEntry *entry,
    app_MemoryLocation *offs, b32 status)
{
    for (app_MemoryLocation *curr_offs = offs; curr_offs; curr_offs = list_next(curr_offs)) {
        curr_offs->last_read_successful = status;
    }

    entry->final_pointer_offset.last_read_successful = status;
}

b32 app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc)
{
    scan_ValueType type = {};

    if (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER) {
        type = SCAN_VALUE_TYPE_POINTER;
    } else {
        type = entry->value_type;
    }

    uintptr_t current_address = entry->base_address.last_resolved_address;
    b32 result = app_addr_table__update_memory_location(&entry->base_address, proc, &current_address, type);

    if (result && (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER)) {
        for (app_MemoryLocation *offs = list_head(&entry->pointer_offsets);
             offs && result;
             offs = list_next(offs)) {
            result = app_addr_table__update_memory_location(
                offs, proc, &current_address, SCAN_VALUE_TYPE_POINTER);

            if (!result) {
                // If one read in the middle of the chain fail, that and all subsequent nodes
                // should be considered as failed.
                app_addr_table_offset_propagate_success_status(entry, offs, false);
            }
        }

        if (result) {
            result = app_addr_table__update_memory_location(
                &entry->final_pointer_offset, proc, &current_address, entry->value_type);
        }
    }

    return result;
}

void app_addr_table_entry_initialize_offset_textbox(app_MemoryLocation *offs)
{
    snprintf(
        offs->offset_textbox,
        array_capacity(offs->offset_textbox),
        "%" PRIuPTR,
        offs->address_offset);
}

void app_addr_table_entry_initialize_textboxes(app_AddressTableEntry *entry)
{
    for (app_MemoryLocation *offs = list_head(&entry->pointer_offsets); offs; offs = list_next(offs)) {
        app_addr_table_entry_initialize_offset_textbox(offs);
    }
}
