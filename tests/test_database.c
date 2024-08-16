#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "../include/database.h"

static int setup(void **state) {
    sqlite3 *db;
    int rc = sqlite3_open(":memory:", &db); // Use in-memory database for testing
    assert_int_equal(rc, SQLITE_OK);

    create_tables(db);
    *state = db;
    return 0;
}

static int teardown(void **state) {
    sqlite3 *db = *state;
    sqlite3_close(db);
    return 0;
}

static void test_insert_project(void **state) {
    sqlite3 *db = *state;
    int project_id = insert_project(db, "Test Project", "1.0");
    assert_true(project_id > 0);

    // Test that the project is not duplicated
    int project_id_dup = insert_project(db, "Test Project", "1.0");
    assert_int_equal(project_id, project_id_dup);
}

static void test_update_database(void **state) {
    sqlite3 *db = *state;
    int project_id = insert_project(db, "Test Project", "1.0");
    assert_true(project_id > 0);

    update_database(db, "Test Dependency", "1.0.0", project_id);

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name, version FROM dependency WHERE name = 'Test Dependency'";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert_int_equal(rc, SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert_int_equal(rc, SQLITE_ROW);
    assert_string_equal((const char*)sqlite3_column_text(stmt, 0), "Test Dependency");
    assert_string_equal((const char*)sqlite3_column_text(stmt, 1), "1.0.0");

    sqlite3_finalize(stmt);
}

static void test_get_existing_dependencies(void **state) {
    sqlite3 *db = *state;
    int project_id = insert_project(db, "Test Project", "1.0");
    assert_true(project_id > 0);

    update_database(db, "Test Dependency", "1.0.0", project_id);

    char **dependencies = NULL;
    int count = 0;
    get_existing_dependencies(db, "Test Project", &dependencies, &count);

    assert_int_equal(count, 1);
    assert_string_equal(dependencies[0], "Test Dependency");

    for (int i = 0; i < count; i++) {
        free(dependencies[i]);
    }
    free(dependencies);
}

static void test_remove_dependency(void **state) {
    sqlite3 *db = *state;
    int project_id = insert_project(db, "Test Project", "1.0");
    assert_true(project_id > 0);

    update_database(db, "Test Dependency", "1.0.0", project_id);

    remove_dependency(db, "Test Dependency", "Test Project");

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name FROM dependency WHERE name = 'Test Dependency'";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert_int_equal(rc, SQLITE_OK);

    rc = sqlite3_step(stmt);
    assert_int_equal(rc, SQLITE_DONE); // No rows should be found

    sqlite3_finalize(stmt);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_insert_project, setup, teardown),
        cmocka_unit_test_setup_teardown(test_update_database, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_existing_dependencies, setup, teardown),
        cmocka_unit_test_setup_teardown(test_remove_dependency, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

