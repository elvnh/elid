#include "proc_object.h"

proc_Object_Maybe proc_get_object_containing_address(proc_ObjectList objects, uintptr_t addr, usize size)
{
    // TODO: providing size is unnecessary?
    proc_Object_Maybe result = {};

    if (ALWAYS(objects.status == PROC_STATUS_OK)) {
        for (usize i = 0; i < objects.count; ++i) {
            proc_Object object = objects.data[i];

            if ((object.base_address <= addr) && (addr < (object.base_address + object.size))) {
                ASSERT(((addr + size) < (object.base_address + object.size)));

                result = SOME(object);
                break;
            }
        }
    }

    return result;
}

proc_Object proc_object_from_memmi_object(Arena *arena, memmi_Object object)
{
    proc_Object result = {};
    result.name = proc_copy_memmi_string(arena, object.path);
    result.base_address = object.base_address;
    result.size = object.size;

    return result;
}

proc_Object proc_object_copy(Arena *arena, proc_Object object)
{
    proc_Object result = {};
    result.name = str_clone(arena, object.name);
    result.base_address = object.base_address;
    result.size = object.size;

    return result;
}

proc_ObjectList proc_object_list_from_memmi_object_list(Arena *arena, memmi_ObjectList objects)
{
    proc_ObjectList result = {};
    result.data = arena_alloc_array(arena, proc_Object, objects.count);
    result.count = objects.count;
    result.status = proc_status_from_memmi_status(objects.status);

    for (usize i = 0; i < objects.count; ++i) {
        result.data[i] = proc_object_from_memmi_object(arena, objects.data[i]);
    }

    return result;
}

b32 proc_object_has_unique_filename(proc_Object object, proc_ObjectList objects)
{
    b32 result = true;

    u32 occurrences = 0;

    String filename_a = path_filename(object.name);

    for (usize i = 0; i < objects.count; ++i) {
        String filename_b = path_filename(objects.data[i].name);

        if (str_eq(filename_a, filename_b)) {
            ++occurrences;

            if (occurrences > 1) {
                result = false;
                break;
            }
        }
    }

    ASSERT(occurrences > 0);
    ASSERT(occurrences <= 2);

    return result;
}

proc_ObjectList proc_get_loaded_objects(Arena *arena, memmi_Process proc)
{
    ScratchArena scratch = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(scratch.arena);

    memmi_ObjectList objects = memmi_get_loaded_objects(proc, temp_allocator);

    proc_ObjectList result = proc_object_list_from_memmi_object_list(arena, objects);

    return result;
}
