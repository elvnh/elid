#pragma once

struct Gui {
    GLFWwindow *window;
};

b32  gui_init(Gui *gui);
b32  gui_should_close(Gui *gui);
void gui_update_and_render(Gui *gui);
