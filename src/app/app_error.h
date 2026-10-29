#pragma once

enum app_ErrorKind {
    APP_ERROR_PROCESS_DIED_BEFORE_OR_DURING_SCAN,
    APP_ERROR_INSUFFICIENT_PERMS_FOR_SCAN,
    APP_ERROR_WRITE_FAILED,
};

struct app_Error {
    app_ErrorKind kind;
};

struct app_ErrorList {
    app_Error data[64];
    usize     count;
    Mutex     mutex;
};

app_ErrorList app_error_list_create();
void          app_error_list_destroy(app_ErrorList *errors);
