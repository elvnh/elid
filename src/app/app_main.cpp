#include "app_main.h"

void app_update_early(app_State *app)
{
    app_action_poll_watch_events(app);
}

void app_update_late(app_State *app)
{
    wq_wait_for_all_tasks(app->per_frame_work_queue);
}
