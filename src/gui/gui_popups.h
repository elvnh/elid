#pragma once

// TODO: gui arg should come first
// TODO: make it possible to have different title for same popup
void gui_popup_open(Arena *arena, gui_State *gui, gui_Popup popup);
void gui_popup_close(gui_State *gui, gui_Popup popup);
void gui_popup_display_active_popups(gui_State *gui, app_State *app);
