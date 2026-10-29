#include "app_error.h"

app_ErrorList app_error_list_create()
{
    app_ErrorList result = {};
    result.mutex = mutex_create();

    return result;
}

void app_error_list_destroy(app_ErrorList *errors)
{
    mutex_destroy(errors->mutex);
}
