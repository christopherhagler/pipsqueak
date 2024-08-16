#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "database.h"
#include "toml.h"

void parse_requirements(const char *project_path, sqlite3 *db, char ***dependencies_list, int *count) {
    char requirements_path[512];
    snprintf(requirements_path, sizeof(requirements_path), "%s/requirements.txt", project_path);

    FILE *file = fopen(requirements_path, "r");
    if (!file) {
        printf("No requirements.txt found at %s\n", requirements_path);
        return;
    }

    // Insert the project into the database (example project entry)
    int project_id = insert_project(db, "example_project", "1.0");

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *name = strtok(line, "==");
        char *version = strtok(NULL, "\n");

        if (name && version) {
            while (*name == ' ' || *name == '\t') name++;
            while (*version == ' ' || *version == '\t') version++;

            update_database(db, name, version, project_id);

            *dependencies_list = realloc(*dependencies_list, (*count + 1) * sizeof(char *));
            (*dependencies_list)[*count] = strdup(name);
            (*count)++;
        }
    }

    fclose(file);
}

void parse_pyproject_toml(const char *project_path, sqlite3 *db, char ***dependencies_list, int *count) {
    char pyproject_path[512];
    snprintf(pyproject_path, sizeof(pyproject_path), "%s/pyproject.toml", project_path);

    FILE *file = fopen(pyproject_path, "r");
    if (!file) {
        printf("No pyproject.toml found at %s\n", pyproject_path);
        return;
    }

    // Load TOML file
    char errbuf[200];
    toml_table_t *config = toml_parse_file(file, errbuf, sizeof(errbuf));
    fclose(file);

    if (!config) {
        printf("TOML parse error: %s\n", errbuf);
        return;
    }

    // Retrieve project name and version
    toml_table_t *tool = toml_table_in(config, "tool");
    toml_table_t *poetry = toml_table_in(tool, "poetry");
    toml_datum_t project_name = toml_string_in(poetry, "name");
    toml_datum_t project_version = toml_string_in(poetry, "version");

    if (!project_name.ok || !project_version.ok) {
        fprintf(stderr, "Error: Project name or version not found in pyproject.toml\n");
        toml_free(config);
        return;
    }

    // Insert project into the database and retrieve its ID
    int project_id = insert_project(db, project_name.u.s, project_version.u.s);
    if (project_id == -1) {
        fprintf(stderr, "Error: Failed to insert project into database\n");
        toml_free(config);
        return;
    }

    // Access dependencies in [tool.poetry.dependencies]
    toml_table_t *poetry_dependencies = toml_table_in(poetry, "dependencies");

    if (poetry_dependencies) {
        for (int i = 0; i < toml_table_nkval(poetry_dependencies); i++) {
            const char *dep_name = toml_key_in(poetry_dependencies, i);
            toml_datum_t dep_version = toml_string_in(poetry_dependencies, dep_name);

            if (dep_version.ok) {
                // Insert dependency linked to the project ID
                update_database(db, dep_name, dep_version.u.s, project_id);

                *dependencies_list = realloc(*dependencies_list, (*count + 1) * sizeof(char *));
                (*dependencies_list)[*count] = strdup(dep_name);
                (*count)++;
            }
        }
    }

    toml_free(config);
}

