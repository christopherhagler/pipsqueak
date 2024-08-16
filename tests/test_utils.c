#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../include/utils.h"

// Dummy function to simulate loading SQL queries from a file
static void test_load_sql_queries(void **state) {
    char buffer[1024] = {0};

    // Call the function to load SQL queries into the buffer
    load_sql_queries(buffer, sizeof(buffer));

    // Test that the buffer is not empty (assuming the file exists and has content)
    assert_true(strlen(buffer) > 0);

    // Example assertion: Check if a specific SQL command is present in the buffer
    assert_non_null(strstr(buffer, "CREATE TABLE IF NOT EXISTS"));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_load_sql_queries),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
