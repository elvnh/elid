#include "app_actions.h"

static void *app__memmi_allocate_cb(void *context, void *ptr, usize old_count, usize new_count, usize item_size, usize align)
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

static memmi_Allocator app__memmi_allocator_from_arena(Arena *arena)
{
    memmi_Allocator result = {};
    result.context = arena;
    result.function = app__memmi_allocate_cb;

    return result;
}

void app_get_running_processes(app_State *app)
{
    arena_reset(app->process_list_arena);

    memmi_Allocator allocator = app__memmi_allocator_from_arena(app->process_list_arena);

    // TODO: do this async, and display empty window in meantime?
    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;
}

b32 app_open_process(app_State *app, memmi_PID pid)
{
    b32 result = false;

    memmi_OpenProcess open_process_result = memmi_open_process(pid);

    if (open_process_result.status != MEMMI_OK) {
        app->open_process_handle = open_process_result.process;
        result = true;
    }

    return result;

}
