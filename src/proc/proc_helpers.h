#pragma once

// This file defines helpers for interacting with the memmi library.

enum proc_Status {
    PROC_STATUS_OK                       = 0,
    PROC_STATUS_NO_SUCH_PROCESS          = (1u << 0u),
    PROC_STATUS_INSUFFICIENT_PERMISSIONS = (1u << 1u),
    PROC_STATUS_PARTIAL_READ_OR_WRITE    = (1u << 2u),
    PROC_STATUS_OTHER_ERROR              = (1u << 3u),
};

struct proc_Object {
    String name;
    usize  base_address;
    usize  size;
};

struct proc_ObjectList {
    proc_Status status;
    proc_Object *data;
    usize count;
};

enum proc_RegionPermission {
    PROC_PERMISSION_READ    = (1u << 0u),
    PROC_PERMISSION_WRITE   = (1u << 1u),
    PROC_PERMISSION_EXECUTE = (1u << 2u),
};
struct proc_MemoryRegion {
    uintptr_t             address;
    usize                 size;
    proc_RegionPermission permissions;
};

struct proc_MemoryRegionList {
    proc_Status status;
    proc_MemoryRegion *data;
    usize count;
};

DEFINE_MAYBE_TYPE(proc_Object);

proc_Status     proc_status_from_memmi_status(memmi_Status status);

// TODO: bad name
proc_ObjectList       proc_get_loaded_objects(Arena *arena, memmi_Process proc);
proc_Object_Maybe     proc_get_object_address_is_contained_in(proc_ObjectList objects, uintptr_t addr, usize size);
memmi_Allocator       proc_memmi_allocator_from_arena(Arena *arena);
String                proc_copy_memmi_string(Arena *arena, memmi_String string);
proc_Object           proc_object_from_memmi_object(Arena *arena, memmi_Object object);
proc_ObjectList       proc_object_list_from_memmi_object_list(Arena *arena, memmi_ObjectList objects);
b32                   proc_object_has_unique_filename(proc_Object object, proc_ObjectList objects);
proc_ObjectList       proc_get_loaded_objects(Arena *arena, memmi_Process proc);
proc_MemoryRegionList proc_get_memory_regions_with_permissions(Arena *arena, memmi_Process proc, proc_RegionPermission perms);
