#ifndef QUERY_H
#define QUERY_H

#include "repl.h"

typedef enum {
    DOT_COMMAND_OK,
    DOT_COMMAND_UNKNOWN
} DotCommandStatus;

typedef enum {
    COMPILE_OK,
    COMPILE_UNKNOWN_QUERY
} CompileStatus;

typedef enum {
    QUERY_INSERT,
    QUERY_SELECT
} QueryKind;

typedef struct {
    QueryKind kind;
} Query;

DotCommandStatus handle_dot_command(LineBuffer *line);
CompileStatus compile_query(LineBuffer *line, Query *query);
void execute_query(Query *query);

#endif /* QUERY_H */
