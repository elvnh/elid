#include "app_command.h"

app_Command *app_command_push(Arena *arena, app_CommandList *list, app_CommandKind kind)
{
    app_Command *result = arena_alloc_struct(arena, app_Command);
    result->kind = kind;

    sll_push_back(list, result);

    return result;
}

static void app__execute_command(app_State *app, app_Command *cmd)
{
    BEGIN_EXHAUSTIVE_SWITCH;
    switch (cmd->kind) {
        case APP_COMMAND_GET_RUNNING_PROCS: {
            TODO("Get running processes");
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
