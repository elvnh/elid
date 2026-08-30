#pragma once

enum gui_Popup {
    /* Normal popups */
    GUI_POPUP_OPEN_PROCESS,

    /* Errors */
    GUI_POPUP_ERROR_PROC_DIED_DURING_SCAN,
    GUI_POPUP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
};

void gui_popup_open(Arena *arena, gui_Popup popup);
void gui_popup_close();
void gui_popup_display_active_popups(gui_State *gui, app_State *app);
