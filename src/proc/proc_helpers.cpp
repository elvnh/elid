#include "proc_helpers.h"

proc_Status proc_status_from_memmi_status(memmi_Status status)
{
    proc_Status result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (status) {
        case MEMMI_OK: {
            result = PROC_STATUS_OK;
        } break;

        case MEMMI_NO_SUCH_PROCESS: {
            result = PROC_STATUS_NO_SUCH_PROCESS;
        } break;

        case MEMMI_INSUFFICIENT_PERMISSIONS: {
            result = PROC_STATUS_INSUFFICIENT_PERMISSIONS;
        } break;

        case MEMMI_INVALID_ARGUMENTS: {
            ASSERT(0);
        } break;

        case MEMMI_ALLOCATION_FAILED: {
            ASSERT_ALWAYS(0);
        } break;

        case MEMMI_PARTIAL_READ_OR_WRITE: {
            result = PROC_STATUS_PARTIAL_READ_OR_WRITE;
        } break;

        case MEMMI_OTHER_ERROR: {
            result = PROC_STATUS_OTHER_ERROR;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

proc_Object_Maybe proc_get_object_address_is_contained_in(proc_ObjectList objects, uintptr_t addr, usize size)
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

static void *proc__memmi_allocate_cb(void *context, void *ptr, usize old_count, usize new_count, usize item_size, usize align)
{
    void *result = 0;
    Arena *arena = (Arena *)context;

    // If count is zero, it's a deallocation, which is a no-op in arenas.
    if (new_count > 0) {
        if (new_count <= old_count) {
            // The library has tried to shrink the allocation, just pass back the old one.
            result = ptr;
        } else {
            result = arena_alloc_bytes(arena, new_count, item_size, align);

            if (result && ptr && (old_count > 0)) {
                // If old count was non-zero, this is a reallocation so we'll have to
                // memcpy the old one over.
                copy_mem(result, ptr, old_count, item_size);
            }
        }
    }

    return result;
}

memmi_Allocator proc_memmi_allocator_from_arena(Arena *arena)
{
    memmi_Allocator result = {};
    result.context = arena;
    result.function = proc__memmi_allocate_cb;

    return result;
}

String proc_copy_memmi_string(Arena *arena, memmi_String string)
{
    String result = str_alloc(arena, string.count);
    copy_mem(result.data, (u8 *)string.data, string.count, sizeof(*string.data));

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
    Temp temp = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(temp.arena);

    memmi_ObjectList objects = memmi_get_loaded_objects(proc, temp_allocator);

    proc_ObjectList result = proc_object_list_from_memmi_object_list(arena, objects);

    return result;
}
