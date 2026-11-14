#include "app_address_table.h"

#define APP_ADDR_TABLE_INIT_NODE_CAP 128

b32 app_addr_table_init(Arena *arena, app_AddressTable *tbl)
{
    b32 result = false;

    *tbl = {};

    tbl->pool = pool_create_untyped(
        arena,
        MAX(sizeof(app_AddressTableEntry), sizeof(app_PointerOffset)),
        APP_ADDR_TABLE_INIT_NODE_CAP,
        MAX(alignof(app_AddressTableEntry), alignof(app_PointerOffset)));

    result = tbl->pool != 0;

    return result;
}

void app_addr_table__delete_entry(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    if (entry) {
        app_PointerOffset *offs = list_head(&entry->offsets);

        while (offs) {
            app_PointerOffset *next = offs->next;

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

    result->offsets = {};

    for (app_PointerOffset *offs = list_head(&entry->offsets); offs; offs = list_next(offs)) {
        app_PointerOffset *offs_copy = pool_alloc(pool, app_PointerOffset);
        *offs_copy = *offs;

        dll_push_back(&result->offsets, offs_copy);
    }

    return result;
}

app_PointerOffset *app_addr_table_entry_push_offset(app_AddressTableEntry *entry, app_PointerOffset *before)
{
    app_PointerOffset *result = pool_alloc(entry->allocated_in_pool, app_PointerOffset);

    if (before) {
        dll_insert_before(&entry->offsets, result, before);
    } else {
        dll_push_back(&entry->offsets, result);
    }

    app_addr_table_entry_initialize_offset_textbox(result);

    return result;
}

void app_addr_table_entry_remove_offset(app_AddressTableEntry *entry, app_PointerOffset *offs)
{
    dll_remove(&entry->offsets, offs);
    pool_free(entry->allocated_in_pool, offs);
}

b32 app_addr_table_entry_swap_offsets(app_PointerOffset *a, app_PointerOffset *b)
{
    b32 result = false;

    if (a && b) {
        result = true;

        app_PointerOffset *a_prev = a->prev;
        app_PointerOffset *a_next = a->next;

        app_PointerOffset *b_prev = b->prev;
        app_PointerOffset *b_next = b->next;

        app_PointerOffset temp = *a;
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
    uintptr_t result = atomic_load_ptr(&entry->address);

    return result;
}

void app_addr_table_entry_set_address(app_AddressTableEntry *entry, uintptr_t value)
{
    atomic_store_ptr(&entry->address, value);
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

    if (entry->kind == APP_ADDR_TABLE_ENTRY_NORMAL) {
        result = entry->last_read_successful;

        if (result) {
            *out = entry->current_value;

        }
    } else {
        ASSERT(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER);
        result = entry->final_offset.last_read_successful;

        if (result) {
            *out = entry->final_offset.last_dereferenced_value;
        }
    }

    return result;

}

b32 app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out)
{
    b32 result = false;

    if (entry->kind == APP_ADDR_TABLE_ENTRY_NORMAL) {
        result = entry->last_read_successful;

        if (result) {
            *out = entry->address;
        }
    } else {
        // For pointer chains, the address of the current node is dependent on the previous
        // node. Therefore, the address is considered valid as long as the read of the previous
        // node succeeded, even if the read of this node failed.
        ASSERT(entry->kind == APP_ADDR_TABLE_ENTRY_POINTER);

        app_PointerOffset *prev_offs = entry->offsets.last;

        if (prev_offs) {
            result = prev_offs->last_read_successful;

            if (result) {
                *out = entry->final_offset.last_resolved_address;
            }
        } else {
            result = entry->last_read_successful;

            if (result) {
                *out = entry->address;
            }
        }
    }

    return result;
}

static b32 app_addr_table__update_pointer_offset(
    app_PointerOffset *offs, memmi_Process proc, uintptr_t *current_address, scan_ValueType type)
{
    b32 result = false;

    offs->last_resolved_address = *current_address;

    result = app_read_single_value_in_process(
        proc,
        *current_address + offs->offset_value,
        type,
        &offs->last_dereferenced_value);

    offs->last_read_successful = result;

    *current_address = offs->last_dereferenced_value.pointer;

    return result;
}

b32 app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc)
{
    scan_ValueType type = {};

    if (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER) {
        type = SCAN_VALUE_TYPE_POINTER;
    } else {
        type = entry->value_type;
    }

    b32 result = app_read_single_value_in_process(
        proc, entry->address, type, &entry->current_value);
    entry->last_read_successful = result;

    if (result && (entry->kind == APP_ADDR_TABLE_ENTRY_POINTER)) {
        uintptr_t current_address = entry->current_value.pointer;

        for (app_PointerOffset *offs = list_head(&entry->offsets);
             offs;
             offs = list_next(offs)) {
            b32 update_ok = app_addr_table__update_pointer_offset(
                offs, proc, &current_address, SCAN_VALUE_TYPE_POINTER);

            result = result && update_ok;
        }

        if (result) {
            result = app_addr_table__update_pointer_offset(
                &entry->final_offset, proc, &current_address, entry->value_type);
        }
    }

    return result;
}

void app_addr_table_entry_initialize_offset_textbox(app_PointerOffset *offs)
{
    snprintf(
        offs->offset_textbox,
        array_capacity(offs->offset_textbox),
        "%" PRIuPTR,
        offs->offset_value);
}

void app_addr_table_entry_initialize_textboxes(app_AddressTableEntry *entry)
{
    for (app_PointerOffset *offs = list_head(&entry->offsets); offs; offs = list_next(offs)) {
        app_addr_table_entry_initialize_offset_textbox(offs);
    }
}
