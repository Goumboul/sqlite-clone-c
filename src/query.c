#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

DotCommandStatus handle_dot_command(LineBuffer *line)
{
    if (strcmp(line->buffer, ".exit") == 0) {
        line_buffer_free(line);
        exit(EXIT_SUCCESS);
    }
    return DOT_COMMAND_UNKNOWN;
}

CompileStatus compile_query(LineBuffer *line, Query *query)
{
    if (strncmp(line->buffer, "insert", 6) == 0) {
        query->kind = QUERY_INSERT;
        return COMPILE_OK;
    }
    if (strcmp(line->buffer, "select") == 0) {
        query->kind = QUERY_SELECT;
        return COMPILE_OK;
    }
    return COMPILE_UNKNOWN_QUERY;
}

void execute_query(Query *query)
{
    switch (query->kind) {
    case QUERY_INSERT:
        printf("This is where we would do an insert.\n");
        break;
    case QUERY_SELECT:
        printf("This is where we would do a select.\n");
        break;
    }
}
