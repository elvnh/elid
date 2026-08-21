#pragma once

// This file defines all actions the application can take. The GUI should only call
// functions defined here.

void app_get_running_processes(app_State *app);
b32  app_open_process(app_State *app, memmi_PID pid);
