#pragma once

void app_update_early(app_State *app)
{
    app->frame_root_task = app_wq_create_task(app->work_queue, app_wq_null_function, app_wq_null_arg);

    app_action_poll_watch_events(app);
}

void app_update_late(app_State *app)
{
    app_wq_run_task(app->frame_root_task);
    app_wq_wait_for_task(app->frame_root_task);

    // TODO: remove this restriction
    ASSERT((app_wq_get_active_task_count(app->work_queue) == 0)
        && "Work is not allowed to span over multiple frames");
}
