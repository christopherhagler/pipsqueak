#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

void create_tables(sqlite3 *db);
int insert_project(sqlite3 *db, const char *name, const char *version);
void update_database(sqlite3 *db, const char *dependency, const char *version, int project_id);
void remove_dependency(sqlite3 *db, const char *dependency, const char *project);
void get_existing_dependencies(sqlite3 *db, const char *project, char ***dependencies, int *count);

#endif // DATABASE_H

