#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

static ExecuteStatus execute_insert(Query *query, Table *table)
{
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    serialize_row(&query->row, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

static ExecuteStatus execute_select(Table *table)
{
    Row row;

    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}

DotCommandStatus handle_dot_command(LineBuffer *line, Table *table)
{
    if (strcmp(line->buffer, ".exit") == 0) {
        table_free(table);
        line_buffer_free(line);
        exit(EXIT_SUCCESS);
    }
    return DOT_COMMAND_UNKNOWN;
}

static CompileStatus compile_insert(LineBuffer *line, Query *query)
{
    strtok(line->buffer, " ");
    char *id_string = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL) {
        return COMPILE_SYNTAX_ERROR;
    }
    if (sscanf(id_string, "%" SCNu32, &query->row.id) != 1) {
        return COMPILE_SYNTAX_ERROR;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return COMPILE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return COMPILE_STRING_TOO_LONG;
    }

    strcpy(query->row.username, username);
    strcpy(query->row.email, email);
    query->kind = QUERY_INSERT;
    return COMPILE_OK;
}

CompileStatus compile_query(LineBuffer *line, Query *query)
{
    if (strncmp(line->buffer, "insert", 6) == 0) {
        return compile_insert(line, query);
    }
    if (strcmp(line->buffer, "select") == 0) {
        query->kind = QUERY_SELECT;
        return COMPILE_OK;
    }
    return COMPILE_UNKNOWN_QUERY;
}

ExecuteStatus execute_query(Query *query, Table *table)
{
    switch (query->kind) {
    case QUERY_INSERT:
        return execute_insert(query, table);
    case QUERY_SELECT:
        return execute_select(table);
    }
    return EXECUTE_SUCCESS;
}
