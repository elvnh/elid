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
            ASSERT_ALWAYS(0);
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

b32 proc_address_is_contained_in_object(memmi_ObjectList objects, uintptr_t addr, usize size)
{
    // TODO: return name of object
    // TODO: providing size is unnecessary?

    b32 result = false;

    if (ALWAYS(objects.status == MEMMI_OK)) {
        for (usize i = 0; i < objects.count; ++i) {
            memmi_Object object = objects.data[i];

            if ((object.base_address <= addr) && (addr < (object.base_address + object.size))) {
                ASSERT(((addr + size) < (object.base_address + object.size)));
                result = true;
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
