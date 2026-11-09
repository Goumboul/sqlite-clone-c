#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"
#include "query.h"

static ExecuteStatus execute_insert(Query *query, Table *table)
{
    void *node = pager_get_page(table->pager, table->root_page_num);
    if (*leaf_node_num_cells(node) >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Cursor *cursor = table_end(table);

    leaf_node_insert(cursor, query->row.id, &query->row);

    free(cursor);

    return EXECUTE_SUCCESS;
}

static ExecuteStatus execute_select(Table *table)
{
    Cursor *cursor = table_start(table);
    Row row;

    while (!cursor->end_of_table) {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }

    free(cursor);

    return EXECUTE_SUCCESS;
}

DotCommandStatus handle_dot_command(LineBuffer *line, Table *table)
{
    if (strcmp(line->buffer, ".exit") == 0) {
        table_close(table);
        line_buffer_free(line);
        exit(EXIT_SUCCESS);
    }
    if (strcmp(line->buffer, ".btree") == 0) {
        printf("Tree:\n");
        print_leaf_node(pager_get_page(table->pager, table->root_page_num));
        return DOT_COMMAND_OK;
    }
    if (strcmp(line->buffer, ".constants") == 0) {
        printf("Constants:\n");
        print_constants();
        return DOT_COMMAND_OK;
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
    char *endptr = NULL;
    errno = 0;
    long id = strtol(id_string, &endptr, 10);
    if (endptr == id_string || *endptr != '\0') {
        return COMPILE_SYNTAX_ERROR;
    }
    if (id < 0) {
        return COMPILE_NEGATIVE_ID;
    }
    if (errno == ERANGE || id > UINT32_MAX) {
        return COMPILE_ID_TOO_LARGE;
    }
    query->row.id = (uint32_t)id;

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
