#pragma once

enum app_AddressTableEntryKind {
    APP_ADDR_TABLE_ENTRY_NORMAL,
    APP_ADDR_TABLE_ENTRY_POINTER,
};

// TODO: allow displaying as signed or in hex
struct app_AddressTableEntry {
    app_AddressTableEntryKind kind;
    scan_ValueType value_type;
    uintptr_t address;        // Atomic
    scan_Value current_value; // Atomic
    b32 last_read_successful; // Atomic


    struct {
        usize offset;

        app_AddressTableEntry *next;
        app_AddressTableEntry *prev;
        app_AddressTableEntry *first;
        app_AddressTableEntry *last;
        usize list_node_count;
    } as_pointer_chain;

    app_AddressTableEntry *next;
    app_AddressTableEntry *prev;
};

struct app_AddressTable {
    Pool *pool;
    app_AddressTableEntry *first;
    app_AddressTableEntry *last;
    usize list_node_count;
};

/* Table operations */
b32    app_addr_table_init(Arena *arena, app_AddressTable *tbl);
void   app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry entry);
void   app_addr_table_remove(app_AddressTable *tbl, app_AddressTableEntry *entry);
void   app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry new_entry);
usize  app_addr_table_count(app_AddressTable *tbl);

/* Entry operations */
uintptr_t app_addr_table_entry_get_address(app_AddressTableEntry *entry);
usize     app_addr_table_entry_get_offset(app_AddressTableEntry *entry);
b32       app_addr_table_entry_try_get_value(app_AddressTableEntry *entry, scan_Value *out);
b32       app_addr_table_entry_try_get_final_value(app_AddressTableEntry *entry, scan_Value *out);
b32       app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out);
b32       app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc);
