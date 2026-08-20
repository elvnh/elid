#include "app_command.h"

app_Command *app_command_push(Arena *arena, app_CommandList *list, app_CommandKind kind)
{
    app_Command *result = arena_alloc_struct(arena, app_Command);
    result->kind = kind;

    sll_push_back(list, result);

    return result;
}

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

memmi_Allocator app__memmi_allocator_from_arena(Arena *arena)
{
    memmi_Allocator result = {};
    result.context = arena;
    result.function = app__memmi_allocate_cb;

    return result;
}

static void app__cmd_get_running_processes(app_State *app)
{
    arena_reset(app->process_list_arena);

    memmi_Allocator allocator = app__memmi_allocator_from_arena(app->process_list_arena);

    // TODO: do this async, and display empty window in meantime?
    memmi_ProcessList processes = memmi_get_running_processes(allocator);
    app->running_process_list = processes;
}

static void app__cmd_open_process(app_State *app, app_Command *cmd)
{
    memmi_OpenProcess open_process_result = memmi_open_process(cmd->as.open_process.pid);

    if (open_process_result.status != MEMMI_OK) {
        TODO("Display popup");
    } else {
        app->open_process_handle = open_process_result.process;
    }
}

static void app__execute_command(app_State *app, app_Command *cmd)
{
    BEGIN_EXHAUSTIVE_SWITCH;
    switch (cmd->kind) {
        case APP_COMMAND_GET_RUNNING_PROCS: {
            app__cmd_get_running_processes(app);
        } break;

        case APP_COMMAND_OPEN_PROCESS: {
            app__cmd_open_process(app, cmd);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;
}

void app_execute_commands(app_State *app, app_CommandList list)
{
    for (app_Command *cmd = list_head(&list); cmd; cmd = list_next(cmd)) {
        app__execute_command(app, cmd);
    }
}
