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

        sll_push_back(tbl, node);
    }
}

usize app_addr_table_count(app_AddressTable *tbl)
{
    usize result = tbl->list_node_count;

    return result;
}

