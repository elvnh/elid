#pragma once

void *allocate_memory(usize size);
void  free_memory(void *ptr, usize size);
usize get_system_page_size(void);
