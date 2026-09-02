#include "base_path.h"

String path_filename(String path)
{
    String result = {};

    // TODO: windows/unix style path separators
    usize_Maybe last_slash_index = str_find_last_occurrence(path, '/');

    if (last_slash_index.ok) {
        result = str_slice_from(path, last_slash_index.value + 1);
    } else {
        result = path;
    }

    return result;
}
