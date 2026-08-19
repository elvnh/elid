#pragma once

struct app_State {
    Arena *frame_arena;
    Arena *process_list_arena;
};

b32 app_init(app_State *app);
