#include "app_state.h"

#define FRAME_ARENA_SIZE        MB(8)
#define PROCESS_LIST_ARENA_SIZE MB(4)

b32 app_init(app_State *app)
{
    b32 result = false;

    app->frame_arena = arena_create(FRAME_ARENA_SIZE);
    app->process_list_arena = arena_create(PROCESS_LIST_ARENA_SIZE);

    result = (app->frame_arena != 0)
        && (app->process_list_arena != 0);

    return result;
}
