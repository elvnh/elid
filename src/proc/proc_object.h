#pragma once

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

DEFINE_MAYBE_TYPE(proc_Object);

proc_ObjectList       proc_get_loaded_objects(Arena *arena, memmi_Process proc);
proc_Object_Maybe     proc_get_object_containing_address(proc_ObjectList objects, uintptr_t addr, usize size);
proc_Object           proc_object_from_memmi_object(Arena *arena, memmi_Object object);
proc_Object           proc_object_copy(Arena *arena, proc_Object object);
proc_ObjectList       proc_object_list_from_memmi_object_list(Arena *arena, memmi_ObjectList objects);
b32                   proc_object_has_unique_filename(proc_Object object, proc_ObjectList objects);
proc_ObjectList       proc_get_loaded_objects(Arena *arena, memmi_Process proc);
