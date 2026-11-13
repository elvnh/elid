#include "app_address_table.h"

#define APP_ADDR_TABLE_INIT_NODE_CAP 1024

b32 app_addr_table_init(Arena *arena, app_AddressTable *tbl)
{
    b32 result = false;

    *tbl = {};

    tbl->pool = pool_create(arena, app_AddressTableEntry, APP_ADDR_TABLE_INIT_NODE_CAP);

    result = tbl->pool != 0;

    return result;
}

void app_addr_table__delete_entry(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    if (entry) {
        app_addr_table__delete_entry(tbl, entry->as_pointer_chain.next);

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

app_AddressTableEntry *app_addr_table_alloc(app_AddressTable *tbl)
{
    app_AddressTableEntry *result = pool_alloc(tbl->pool, app_AddressTableEntry);

    return result;
}

void app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    dll_push_back(tbl, entry);
}

void app_addr_table_remove(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    entry->flagged_for_deletion = true;
}

void app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry *new_entry)
{
    // Replace ourselves with the new node.
    if (entry->next) {
        entry->next->prev = new_entry;
    }

    if (entry->prev) {
        entry->prev->next = new_entry;
    }

    if (tbl->first == entry) {
        tbl->first = new_entry;
    }

    if (tbl->last == entry) {
        tbl->first = new_entry;
    }

    app_addr_table_remove(tbl, entry);
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

    result->next = 0;
    result->prev = 0;

    if (result->as_pointer_chain.next) {
        ASSERT(result->kind == APP_ADDR_TABLE_ENTRY_POINTER);

        app_AddressTableEntry *next = app_addr_table_entry_clone(pool, entry->as_pointer_chain.next);
        dll_push_back(&result->as_pointer_chain, next);

        ASSERT(next->kind == APP_ADDR_TABLE_ENTRY_POINTER);
    }

    return result;
}

uintptr_t app_addr_table_entry_get_address(app_AddressTableEntry *entry)
{
    uintptr_t result = atomic_load_ptr(&entry->address);

    return result;
}

void app_addr_table_entry_set_address(app_AddressTableEntry *entry, uintptr_t value)
{
    atomic_store_ptr(&entry->address, value);
}

usize app_addr_table_entry_get_offset(app_AddressTableEntry *entry)
{
    usize result = atomic_load_ptr(&entry->as_pointer_chain.offset);

    return result;
}

void app_addr_table_entry_set_offset(app_AddressTableEntry *entry, usize value)
{
    atomic_store_usize(&entry->as_pointer_chain.offset, value);
}

b32 app_addr_table_entry_try_get_value(app_AddressTableEntry *entry, scan_Value *out)
{
    b32 result = atomic_load_s32(&entry->last_read_successful);

    if (result) {
        scan_val_atomic_move(out, &entry->current_value);
    }

    return result;
}

b32 app_addr_table_entry_try_get_final_value(app_AddressTableEntry *entry, scan_Value *out)
{
    b32 result = false;

    scan_Value value = {};

    for (app_AddressTableEntry *e = entry; e; e = list_next(&e->as_pointer_chain)) {
        result = app_addr_table_entry_try_get_value(e, &value);

        if (!result) {
            break;
        }
    }

    if (result) {
        *out = value;
    }

    return result;

}

b32 app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out)
{
    b32 result = false;

    uintptr_t address = 0;

    for (app_AddressTableEntry *e = entry; e; e = list_next(&e->as_pointer_chain)) {
        result = atomic_load_s32(&e->last_read_successful);
        address = atomic_load_ptr(&e->address);

        if (!result) {
            break;
        }
    }

    if (result) {
        *out = address;
    }

    return result;
}

// TODO: better name
b32 app_addr_table_entry_update_value_no_chain(app_AddressTableEntry *entry, memmi_Process proc, scan_Value *opt_value)
{
    b32 result = false;

    scan_Value value = {};
    scan_ValueType value_type = entry->value_type;

    if (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER) {
        // If this is a link in a pointer chain, we want to read the value as a pointer.
        value_type = SCAN_VALUE_TYPE_POINTER;
    }

    result = app_read_single_value_in_process(proc, entry->address, value_type, &value);

    atomic_store_s32(&entry->last_read_successful, result);

    if (result) {
        scan_val_atomic_store(&entry->current_value, value);

        if (opt_value) {
            *opt_value = value;
        }
    }

    return result;
}

b32 app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc)
{
    scan_Value current_value = {};

    b32 result = app_addr_table_entry_update_value_no_chain(entry, proc, &current_value);

    if (result && entry->as_pointer_chain.next) {
        ALWAYS(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER);

        // The value found at this address, plus the offset, becomes the address of the next link in
        // the pointer chain.
        app_AddressTableEntry *next_entry = entry->as_pointer_chain.next;

        usize offset = app_addr_table_entry_get_offset(entry);

        uintptr_t address_plus_offset = 0;

        if (safe_add_usize(current_value.pointer, offset, &address_plus_offset)) {
            app_addr_table_entry_set_offset(next_entry, address_plus_offset);
        }

        result = app_addr_table_entry_update_value(next_entry, proc);
    }

    return result;
}

void app_addr_table_entry_initialize_textboxes(app_AddressTableEntry *entry)
{
    for (app_AddressTableEntry *e = entry; e; e = list_next(&e->as_pointer_chain)) {
        uintptr_t address = app_addr_table_entry_get_address(e);
        usize offset = app_addr_table_entry_get_offset(e);

        snprintf(e->gui_input.address_textbox, array_capacity(e->gui_input.address_textbox), "%" PRIxPTR, address);
        snprintf(e->gui_input.offset_textbox,array_capacity(e->gui_input.offset_textbox), "%" PRIuPTR, offset);
    }
}
