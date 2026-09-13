#define TEST_WQ_REPEAT_COUNT 10

TEST_CASE_REPEAT(wq_basic, TEST_WQ_REPEAT_COUNT)
{
    Temp temp = temp_begin(test_arena);

    app_WorkQueue *wq = app_wq_create(temp.arena, 16);

    int i = 0;
    int *i_ptr = &i;

    app_TaskID task = app_wq_create_task(wq, [] (void *data) {
        int *ptr = *(int **)data;
        *ptr = 123;
    }, i_ptr);

    REQUIRE(i == 0);

    app_wq_run_task(task);
    app_wq_wait_for_task(task);

    REQUIRE(i == 123);

    app_wq_destroy(wq);
}

TEST_CASE(wq_basic_destroy_no_wait)
{
    Temp temp = temp_begin(test_arena);

    app_WorkQueue *wq = app_wq_create(temp.arena, 16);

    int i = 0;
    int *i_ptr = &i;

    app_TaskID task = app_wq_create_task(wq, [] (void *data) {
        int *ptr = *(int **)data;
        *ptr = 123;
    }, i_ptr);

    REQUIRE(i == 0);

    app_wq_run_task(task);
    sleep_ms(100);
    app_wq_destroy(wq);

    REQUIRE(i == 123);
}

#define TEST_WQ_DEFAULT_WAIT_MS 100

TEST_CASE_REPEAT(wq_child_task_wait_for_parent_with_slow_child, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        Temp temp = temp_begin(test_arena);
        app_WorkQueue *wq = app_wq_create(temp.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        app_TaskID parent = app_wq_create_task(wq, [] (void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        app_TaskID child = app_wq_create_task_as_child(wq, [] (void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            app_wq_run_task(child);
            app_wq_run_task(parent);
        } else {
            app_wq_run_task(parent);
            app_wq_run_task(child);
        }


        app_wq_wait_for_task(parent);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        app_wq_destroy(wq);
    }


}

TEST_CASE_REPEAT(wq_child_task_wait_for_slow_child, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        Temp temp = temp_begin(test_arena);
        app_WorkQueue *wq = app_wq_create(temp.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        app_TaskID parent = app_wq_create_task(wq, [] (void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        app_TaskID child = app_wq_create_task_as_child(wq, [] (void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            app_wq_run_task(child);
            app_wq_run_task(parent);
        } else {
            app_wq_run_task(parent);
            app_wq_run_task(child);
        }

        app_wq_wait_for_task(child);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        app_wq_destroy(wq);
    }

}

TEST_CASE_REPEAT(wq_child_task_wait_for_slow_parent, TEST_WQ_REPEAT_COUNT)
{
    for (s32 i = 0; i <= 1; ++i) {
        b32 reverse = i == 1;

        Temp temp = temp_begin(test_arena);
        app_WorkQueue *wq = app_wq_create(temp.arena, 16);

        s32 counter = 0;
        s32 *counter_ptr = &counter;

        app_TaskID parent = app_wq_create_task(wq, [] (void *data) {
            sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr);

        app_TaskID child = app_wq_create_task_as_child(wq, [] (void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        u64 tp1 = get_monotonic_clock_ms();

        if (reverse) {
            app_wq_run_task(child);
            app_wq_run_task(parent);
        } else {
            app_wq_run_task(parent);
            app_wq_run_task(child);
        }

        app_wq_wait_for_task(parent);

        u64 tp2 = get_monotonic_clock_ms();
        u64 elapsed = tp2 - tp1;

        REQUIRE(counter == 2);

        // Check that the longer task was waited for.
        REQUIRE(elapsed >= TEST_WQ_DEFAULT_WAIT_MS);

        app_wq_destroy(wq);
    }

}

TEST_CASE_REPEAT(wq_wait_for_finished_task, TEST_WQ_REPEAT_COUNT)
{
    Temp temp = temp_begin(test_arena);
    app_WorkQueue *wq = app_wq_create(temp.arena, 16);

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    app_TaskID task = app_wq_create_task(wq, [] (void *data) {
        sleep_ms(TEST_WQ_DEFAULT_WAIT_MS);

        s32 *ptr = *(s32 **)data;
        atomic_inc_s32(ptr);
    }, counter_ptr);

    app_wq_run_task(task);

    // Busy loop until task has finished.
    while (atomic_load_s32(&counter) == 0);

    REQUIRE(atomic_load_s32(&counter) == 1);

    app_wq_wait_for_task(task);
    REQUIRE(atomic_load_s32(&counter) == 1);

    app_wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_many_tasks, TEST_WQ_REPEAT_COUNT)
{
    Temp temp = temp_begin(test_arena);
    app_WorkQueue *wq = app_wq_create(temp.arena, 16);

    s32 n = 1000;

    s32 counter = 0;
    s32 *counter_ptr = &counter;

    app_TaskID parent = app_wq_create_task(wq, app_wq_null_function, app_wq_null_arg);

    for (s32 i = 0; i < n; ++i) {
        app_TaskID task = app_wq_create_task_as_child(wq, [] (void *data) {
            s32 *ptr = *(s32 **)data;
            atomic_inc_s32(ptr);
        }, counter_ptr, parent);

        app_wq_run_task(task);
    }

    // TODO: function with run and wait combined?
    app_wq_run_task(parent);
    app_wq_wait_for_task(parent);

    REQUIRE(counter == n);

    app_wq_destroy(wq);
}

TEST_CASE_REPEAT(wq_destroy_before_finish, TEST_WQ_REPEAT_COUNT)
{
    Temp temp = temp_begin(test_arena);
    app_WorkQueue *wq = app_wq_create(temp.arena, 16);

    app_TaskID task = app_wq_create_task(wq, app_wq_null_function, app_wq_null_arg);

    app_wq_run_task(task);

    app_wq_destroy(wq);
}

