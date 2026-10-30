#pragma once

typedef int64_t Pid;

void *allocate_memory(usize size);
void  free_memory(void *ptr, usize size);
usize get_system_page_size(void);
void  sleep_ms(u32 ms);
Pid   get_self_pid();
