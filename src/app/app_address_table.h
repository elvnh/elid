#pragma once

// TODO: make sure concurrent updated to address table don't happen

enum app_AddressTableEntryKind {
    APP_ADDR_TABLE_ENTRY_NORMAL,
    APP_ADDR_TABLE_ENTRY_POINTER,
};

struct app_MemoryLocation {
    uintptr_t            last_resolved_address;
    size_t               address_offset;
    scan_Value           last_dereferenced_value;
    b32                  last_read_successful;

    app_MemoryLocation  *next;
    app_MemoryLocation  *prev;

    char offset_textbox[64];
};

struct app_MemoryLocationList {
    app_MemoryLocation *first;
    app_MemoryLocation *last;
    usize list_node_count;
};

// TODO: allow displaying as signed or in hex
struct app_AddressTableEntry {
    app_AddressTableEntryKind kind;
    scan_ValueType            value_type;

    app_MemoryLocation     base_address;

    app_MemoryLocationList pointer_offsets;
    app_MemoryLocation     final_pointer_offset; // The extra node all pointer chains have at the end.

    // We don't want to actually entries mid-frame while iterating
    // through the table, so we defer actual deletion until end of frames.
    b32 flagged_for_deletion;

    Pool *allocated_in_pool;

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
b32                    app_addr_table_init(Arena *arena, app_AddressTable *tbl);
void                   app_addr_table_delete_flagged_entries(app_AddressTable *tbl);
void                   app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry *entry);
void                   app_addr_table_remove(app_AddressTableEntry *entry);
void                   app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry *new_entry);
usize                  app_addr_table_count(app_AddressTable *tbl);

/* Entry operations */
app_AddressTableEntry *app_addr_table_entry_alloc(Pool *pool);
app_AddressTableEntry *app_addr_table_entry_clone(Pool *pool, app_AddressTableEntry *entry);
app_MemoryLocation     *app_addr_table_entry_push_offset(app_AddressTableEntry *entry, app_MemoryLocation *before);
void                   app_addr_table_entry_remove_offset(app_AddressTableEntry *entry, app_MemoryLocation *offs);
b32                    app_addr_table_entry_swap_offsets(app_MemoryLocation *a, app_MemoryLocation *b);
void                   app_addr_table_offset_propagate_success_status(app_AddressTableEntry *entry,
                                                                      app_MemoryLocation *offs, b32 status);

b32      			   app_addr_table_entry_try_get_value(app_AddressTableEntry *entry, scan_Value *out);
b32      			   app_addr_table_entry_try_get_final_value(app_AddressTableEntry *entry, scan_Value *out);
b32      			   app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out);
b32      			   app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc);
void     			   app_addr_table_entry_initialize_textboxes(app_AddressTableEntry *entry);
void     			   app_addr_table_entry_initialize_offset_textbox(app_MemoryLocation *offs);
