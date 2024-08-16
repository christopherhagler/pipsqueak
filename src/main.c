#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <getopt.h>
#include <string.h>
#include "database.h"
#include "parser.h"

// Command-line options
struct option long_options[] = {
    {"project", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char *argv[]) {
    // Parse command-line options
    int option_index = 0;
    char *project_path = NULL;
    int opt;

    while ((opt = getopt_long(argc, argv, "p:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                project_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s --project <path>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (project_path == NULL) {
        fprintf(stderr, "Usage: %s --project <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the SQLite database
    sqlite3 *db;
    int rc = sqlite3_open("dependencies.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Create tables if they don't exist
    create_tables(db);

    // Get existing dependencies from the database for this project
    char **existing_dependencies = NULL;
    int existing_count = 0;
    get_existing_dependencies(db, project_path, &existing_dependencies, &existing_count);

    // Track current dependencies
    char **current_dependencies = NULL;
    int current_count = 0;

    // Check if pyproject.toml exists
    char pyproject_path[512];
    snprintf(pyproject_path, sizeof(pyproject_path), "%s/pyproject.toml", project_path);

    FILE *pyproject_file = fopen(pyproject_path, "r");
    if (pyproject_file) {
        // If pyproject.toml exists, parse it
        fclose(pyproject_file);
        parse_pyproject_toml(project_path, db, &current_dependencies, &current_count);
    } else {
        // Otherwise, parse requirements.txt
        parse_requirements(project_path, db, &current_dependencies, &current_count);
    }

    // Remove dependencies that are no longer present in the project
    for (int i = 0; i < existing_count; i++) {
        int found = 0;
        for (int j = 0; j < current_count; j++) {
            if (strcmp(existing_dependencies[i], current_dependencies[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            remove_dependency(db, existing_dependencies[i], project_path);
        }
    }

    // Free dynamically allocated memory
    free(existing_dependencies);
    free(current_dependencies);

    // Close the database connection
    sqlite3_close(db);

    return 0;
}
