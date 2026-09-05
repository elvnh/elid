#pragma once

struct Thread {
    u64 data;
};

DEFINE_MAYBE_TYPE(Thread);

typedef void *(*ThreadEntryPoint)(void *user_data);

Thread_Maybe thread_create(ThreadEntryPoint entry_point, void *user_data);
b32          thread_join(Thread thread);
