#include <math.h>

#define TEST_WQ_REPEAT_COUNT 50

TEST_CASE_REPEAT(wq_basic, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();

    WorkQueue *wq = wq_create(scratch.arena, 16);

    int i = 0;
    int *i_ptr = &i;

    TaskID task = wq_create_task(wq, [] (TaskID, void *data) {
        int *ptr = *(int **)data;
        *ptr = 123;
    }, i_ptr);

    REQUIRE(i == 0);

    wq_run_task(task);
    wq_wait_for_task(task);

    REQUIRE(i == 123);

    wq_destroy(wq);
}

TEST_CASE(wq_basic_destroy_no_wait)
{
    ScratchArena scratch = scratch_begin();

    WorkQueue *wq = wq_create(scratch.arena, 16);

    int i = 0;
    int *i_ptr = &i;

    TaskID task = wq_create_task(wq, [] (TaskID, void *data) {
        int *ptr = *(int **)data;
        *ptr = 123;
    }, i_ptr);

    REQUIRE(i == 0);

    wq_run_task(task);
    sleep_ms(100);

    REQUIRE(i == 123);

    wq_destroy(wq);
}

#define TEST_WQ_DEFAULT_WAIT_MS 100

TEST_CASE_REPEAT(wq_child_task_wait_for_parent_with_slow_child, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        ScratchArena scratch = scratch_begin();
        WorkQueue *wq = wq_create(scratch.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        TaskID parent = wq_create_task(wq, [] (TaskID, void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        TaskID child = wq_create_task_as_child(wq, [] (TaskID, void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            wq_run_task(child);
            wq_run_task(parent);
        } else {
            wq_run_task(parent);
            wq_run_task(child);
        }


        wq_wait_for_task(parent);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        wq_destroy(wq);
    }
}

TEST_CASE_REPEAT(wq_child_task_wait_for_slow_child, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        ScratchArena scratch = scratch_begin();
        WorkQueue *wq = wq_create(scratch.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        TaskID parent = wq_create_task(wq, [] (TaskID, void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        TaskID child = wq_create_task_as_child(wq, [] (TaskID, void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            wq_run_task(child);
            wq_run_task(parent);
        } else {
            wq_run_task(parent);
            wq_run_task(child);
        }

        wq_wait_for_task(child);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        wq_wait_for_task(parent);

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        wq_destroy(wq);
    }

}

TEST_CASE_REPEAT(wq_child_task_wait_for_slow_parent, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        ScratchArena scratch = scratch_begin();
        WorkQueue *wq = wq_create(scratch.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        TaskID parent = wq_create_task(wq, [] (TaskID, void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        TaskID child = wq_create_task_as_child(wq, [] (TaskID, void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            wq_run_task(child);
            wq_run_task(parent);
        } else {
            wq_run_task(parent);
            wq_run_task(child);
        }

        wq_wait_for_task(parent);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        wq_destroy(wq);
    }

}

TEST_CASE_REPEAT(wq_wait_for_finished_task, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    TaskID task = wq_create_task(wq, [] (TaskID, void *data) {
        sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

        s32 *ptr = *(s32 **)data;
        atomic_inc_s32(ptr);
    }, counter_ptr);

    wq_run_task(task);

    // Busy loop until task has finished.
    while (atomic_load_s32(&counter) == 0);

    REQUIRE(atomic_load_s32(&counter) == 1);

    wq_wait_for_task(task);
    REQUIRE(atomic_load_s32(&counter) == 1);

    wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_many_tasks, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 n = 1000;

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    TaskID parent = wq_create_task(wq, wq_null_function, wq_null_arg);

    for (s32 i = 0; i < n; ++i) {
        TaskID task = wq_create_task_as_child(wq, [] (TaskID, void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        wq_run_task(task);
    }

    // TODO: function with run and wait combined?
    wq_run_task(parent);
    wq_wait_for_task(parent);

    REQUIRE(counter == n);

    wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_destroy_before_finish, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    TaskID task = wq_create_task(wq, wq_null_function, wq_null_arg);

    wq_run_task(task);

    wq_destroy(wq);
}

struct TaskTreeHelperArgs {
    WorkQueue *wq;
    s32 depth;
    s32 max_depth;
    s32 *counter;
};

static void test_wq_deep_task_tree_helper(TaskID self, void *data)
{
    TaskTreeHelperArgs *args = (TaskTreeHelperArgs *)data;

    if (args->depth < args->max_depth) {
        TaskTreeHelperArgs new_args = *args;
        ++new_args.depth;

        TaskID left = wq_create_task_as_child(args->wq, test_wq_deep_task_tree_helper, new_args, self);
        TaskID right = wq_create_task_as_child(args->wq, test_wq_deep_task_tree_helper, new_args, self);

        wq_run_task(left);
        wq_run_task(right);
    } else {
        atomic_inc_s32(args->counter);
    }
}

TEST_CASE_REPEAT(wq_deep_task_tree, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 counter = 0;

    TaskTreeHelperArgs args = {};
    args.wq = wq;
    args.max_depth = 10;
    args.counter = &counter;

    TaskID task = wq_create_task(wq, test_wq_deep_task_tree_helper, args);

    wq_run_task(task);
    wq_wait_for_task(task);

    // The tasks form a complete binary tree where every leaf node increases the counter once.
    REQUIRE(counter == pow(2, args.max_depth));

    wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_wait_for_all_tasks_single_task, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    wq_push_task(wq, [] (TaskID, void *data) {
        s32 *c = *(s32 **)data;

        sleep_ms(50);

        *c += 1;
    }, counter_ptr);

    wq_wait_for_all_tasks(wq);

    REQUIRE(counter == 1);

    wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_wait_for_all_tasks_multiple_tasks, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    s32 counter_target = 100;

    for (s32 i = 0; i < counter_target; ++i) {
        wq_push_task(wq, [] (TaskID, void *data) {
            s32 *c = *(s32 **)data;

            sleep_ms(50);

            atomic_inc_s32(c);
        }, counter_ptr);
    }

    wq_wait_for_all_tasks(wq);

    REQUIRE(counter == counter_target);

    wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_wait_for_all_tasks_multiple_tasks_different_durations, TEST_WQ_REPEAT_COUNT)
{
    ScratchArena scratch = scratch_begin();
    WorkQueue *wq = wq_create(scratch.arena, 16);

    s32 counter = 0;

    s32 counter_target = 100;

    struct Args {
        s32 *counter_ptr;
        s32 wait_duration_in_ms;
    };

    for (s32 i = 0; i < counter_target; ++i) {
        Args args = {&counter, i * 10};

        wq_push_task(wq, [] (TaskID, void *data) {
            Args a = *(Args *)data;

            sleep_ms((u32)a.wait_duration_in_ms);

            atomic_inc_s32(a.counter_ptr);
        }, args);
    }

    wq_wait_for_all_tasks(wq);

    REQUIRE(counter == counter_target);

    wq_destroy(wq);
}
