#ifndef QUERY_H
#define QUERY_H

#include "repl.h"
#include "row.h"
#include "table.h"

typedef enum {
    DOT_COMMAND_OK,
    DOT_COMMAND_UNKNOWN
} DotCommandStatus;

typedef enum {
    COMPILE_OK,
    COMPILE_UNKNOWN_QUERY,
    COMPILE_SYNTAX_ERROR,
    COMPILE_STRING_TOO_LONG,
    COMPILE_NEGATIVE_ID,
    COMPILE_ID_TOO_LARGE
} CompileStatus;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteStatus;

typedef enum {
    QUERY_INSERT,
    QUERY_SELECT
} QueryKind;

typedef struct {
    QueryKind kind;
    Row row;
} Query;

DotCommandStatus handle_dot_command(LineBuffer *line, Table *table);
CompileStatus compile_query(LineBuffer *line, Query *query);
ExecuteStatus execute_query(Query *query, Table *table);

#endif /* QUERY_H */
