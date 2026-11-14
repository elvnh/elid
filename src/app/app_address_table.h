#pragma once

// TODO: make sure concurrent updated to address table don't happen

enum app_AddressTableEntryKind {
    APP_ADDR_TABLE_ENTRY_NORMAL,
    APP_ADDR_TABLE_ENTRY_POINTER,
};

// TODO: try to unify AddressTableEntry and PointerOffset, since they're so similar
struct app_PointerOffset {
    size_t              offset_value;
    b32                 last_read_successful;
    uintptr_t           last_resolved_address;
    scan_Value          last_dereferenced_value;
    app_PointerOffset  *next;
    app_PointerOffset  *prev;

    char offset_textbox[64];
};

struct app_PointerOffsets {
    app_PointerOffset *first;
    app_PointerOffset *last;
    usize list_node_count;
};

// TODO: allow displaying as signed or in hex
struct app_AddressTableEntry {
    app_AddressTableEntryKind kind; // TODO: unnecessary?
    scan_ValueType value_type;
    uintptr_t address;        // Atomic

    // Only updated asynchronously by background thread.
    // TODO: rename to last_known_value
    scan_Value current_value; // Atomic
    b32 last_read_successful; // Atomic



    app_PointerOffsets offsets;
    app_PointerOffset  final_offset; // The extra node all pointer chains have at the end.

    app_AddressTableEntry *next;
    app_AddressTableEntry *prev;
    // TODO: pack into struct called table_member

    // We don't want to actually delete entries mid-frame while iterating
    // through the table, so we defer actual deletion until end of frames.
    b32 flagged_for_deletion;
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
app_AddressTableEntry *app_addr_table_entry_alloc(Pool *pool);
void                   app_addr_table_push(app_AddressTable *tbl, app_AddressTableEntry *entry);
void                   app_addr_table_remove(app_AddressTable *tbl, app_AddressTableEntry *entry);
void                   app_addr_table_change(app_AddressTable *tbl, app_AddressTableEntry *entry, app_AddressTableEntry *new_entry);
usize                  app_addr_table_count(app_AddressTable *tbl);

/* Entry operations */
app_AddressTableEntry *app_addr_table_entry_clone(Pool *pool, app_AddressTableEntry *entry);
app_PointerOffset     *app_addr_table_entry_push_offset(app_AddressTable *tbl, app_AddressTableEntry *entry);

// TODO: remove these
uintptr_t			   app_addr_table_entry_get_address(app_AddressTableEntry *entry);
void     			   app_addr_table_entry_set_address(app_AddressTableEntry *entry, uintptr_t value);
void     			   app_addr_table_entry_set_offset(app_AddressTableEntry *entry, usize value);

void     			   app_addr_table_entry_set_type(app_AddressTableEntry *entry, scan_ValueType type);
/// TODO

b32      			   app_addr_table_entry_try_get_value(app_AddressTableEntry *entry, scan_Value *out);
b32      			   app_addr_table_entry_try_get_final_value(app_AddressTableEntry *entry, scan_Value *out);
b32      			   app_addr_table_entry_try_get_final_address(app_AddressTableEntry *entry, uintptr_t *out);
b32      			   app_addr_table_entry_update_value(app_AddressTableEntry *entry, memmi_Process proc);
void     			   app_addr_table_entry_initialize_textboxes(app_AddressTableEntry *entry);
void     			   app_addr_table_entry_initialize_offset_textbox(app_PointerOffset *offs);
