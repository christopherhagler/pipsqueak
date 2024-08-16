#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

// Function to load SQL queries from a file
void load_sql_queries(char *buffer, size_t buffer_size) {
    FILE *file = fopen("sql/queries.sql", "r");
    if (!file) {
        fprintf(stderr, "Failed to open SQL queries file.\n");
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, buffer_size, file);
    fclose(file);
}
