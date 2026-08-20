#pragma once

enum app_CommandKind {
    APP_COMMAND_GET_RUNNING_PROCS,
    APP_COMMAND_OPEN_PROCESS,
};

struct app_Command {
    app_CommandKind kind;

    union {
        struct {
            memmi_PID pid;
        } open_process;
    } as;

    app_Command *next;
};

struct app_CommandList {
    app_Command *first;
    app_Command *last;
};

app_Command *app_command_push(Arena *arena, app_CommandList *list, app_CommandKind kind);
void         app_execute_commands(app_State *app, app_CommandList list);
