#pragma once

// This file defines all actions the application can take. The GUI should only call
// functions defined here.
void app_action_get_running_processes(app_State *app);
b32  app_action_open_process(app_State *app, memmi_PID pid);
void app_action_close_opened_process(app_State *app);
void app_action_run_first_scan(app_State *app, scan_Comparison cmp, usize stride);
void app_action_run_next_scan(app_State *app, scan_Comparison cmp);
