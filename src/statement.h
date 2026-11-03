#ifndef STATEMENT_H
#define STATEMENT_H

#include "repl.h"

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct {
    StatementType type;
} Statement;

MetaCommandResult do_meta_command(LineBuffer *line);
PrepareResult prepare_statement(LineBuffer *line, Statement *statement);
void execute_statement(Statement *statement);

#endif /* STATEMENT_H */
