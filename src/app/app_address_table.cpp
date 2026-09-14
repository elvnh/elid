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

void app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry entry)
{
    app_AddressTableEntry *node = pool_alloc(tbl->pool, app_AddressTableEntry);

    if (ALWAYS(node)) {
        *node = entry;

        dll_push_back(tbl, node);
    }
}

void app_addr_table_remove(app_AddressTable *tbl, app_AddressTableEntry *entry)
{
    dll_remove(tbl, entry);
}

void app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry new_entry)
{
    (void)tbl;

    // We only change some of these members since others will be updated atomically by other threads.
    entry->address = new_entry.address;
    entry->value_type = new_entry.value_type;
}

usize app_addr_table_count(app_AddressTable *tbl)
{
    usize result = tbl->list_node_count;

    return result;
}
