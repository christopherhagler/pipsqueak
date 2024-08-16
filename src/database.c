#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"
#include "utils.h"

void create_tables(sqlite3 *db) {
    char sql[1024];
    load_sql_queries(sql, sizeof(sql));
    sqlite3_exec(db, sql, 0, 0, NULL);
}

int get_project_id(sqlite3 *db, const char *name, const char *version) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM project WHERE name = ? AND version = ?";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, version, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int project_id = -1;
    if (rc == SQLITE_ROW) {
        project_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return project_id;
}

int insert_project(sqlite3 *db, const char *name, const char *version) {
    int project_id = get_project_id(db, name, version);
    if (project_id != -1) {
        // Project already exists, return its ID
        return project_id;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO project (name, version, used_in_production) VALUES (?, ?, 0);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, version, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert project: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    project_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return project_id;
}

void update_database(sqlite3 *db, const char *dependency, const char *version, int project_id) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT OR REPLACE INTO dependency (name, version, project_id) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dependency, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, version, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, project_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert dependency: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void get_existing_dependencies(sqlite3 *db, const char *project, char ***dependencies, int *count) {
    const char *sql = "SELECT name FROM dependency WHERE project_id = (SELECT id FROM project WHERE name = ?)";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, project, -1, SQLITE_STATIC);

    *count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        *dependencies = realloc(*dependencies, (*count + 1) * sizeof(char *));
        (*dependencies)[*count] = strdup((const char *)sqlite3_column_text(stmt, 0));
        (*count)++;
    }

    sqlite3_finalize(stmt);
}

void remove_dependency(sqlite3 *db, const char *dependency, const char *project) {
    const char *sql = "DELETE FROM dependency WHERE name = ? AND project_id = (SELECT id FROM project WHERE name = ?)";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dependency, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, project, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to remove dependency: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

