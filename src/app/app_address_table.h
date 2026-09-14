#pragma once

struct app_AddressTableEntry {
    // TODO: allow displaying as signed or in hex
    uintptr_t address;
    scan_ValueType value_type;
    scan_Value current_value;
    b32 last_read_successful;

    app_AddressTableEntry *next;
    app_AddressTableEntry *prev;
};

struct app_AddressTable {
    Pool *pool;
    app_AddressTableEntry *first;
    app_AddressTableEntry *last;
    usize list_node_count;
};

b32    app_addr_table_init(Arena *arena, app_AddressTable *tbl);
void   app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry entry);
void   app_addr_table_remove(app_AddressTable *tbl, app_AddressTableEntry *entry);
usize  app_addr_table_count(app_AddressTable *tbl);
