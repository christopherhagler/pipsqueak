#ifndef PARSER_H
#define PARSER_H

#include <sqlite3.h>

// Function prototypes
void parse_requirements(const char *project_path, sqlite3 *db, char ***dependencies, int *count);
void parse_pyproject_toml(const char *project_path, sqlite3 *db, char ***dependencies, int *count);

#endif // PARSER_H

