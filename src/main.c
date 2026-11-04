#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "query.h"
#include "repl.h"
#include "table.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }

    LineBuffer *line = line_buffer_create();
    Table *table = table_open(argv[1]);

    while (true) {
        show_prompt();
        if (!read_line(line)) {
            printf("Error reading input\n");
            table_close(table);
            line_buffer_free(line);
            exit(EXIT_FAILURE);
        }

        if (line->buffer[0] == '.') {
            if (handle_dot_command(line, table) == DOT_COMMAND_UNKNOWN) {
                printf("Command not found: '%s'.\n", line->buffer);
            }
            continue;
        }

        Query query;
        switch (compile_query(line, &query)) {
        case COMPILE_OK:
            break;
        case COMPILE_UNKNOWN_QUERY:
            printf("Unrecognized keyword at start of '%s'.\n", line->buffer);
            continue;
        case COMPILE_SYNTAX_ERROR:
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case COMPILE_STRING_TOO_LONG:
            printf("String is too long.\n");
            continue;
        case COMPILE_NEGATIVE_ID:
            printf("ID must be positive.\n");
            continue;
        case COMPILE_ID_TOO_LARGE:
            printf("ID is too large.\n");
            continue;
        }

        switch (execute_query(&query, table)) {
        case EXECUTE_SUCCESS:
            printf("Executed.\n");
            break;
        case EXECUTE_TABLE_FULL:
            printf("Error: Table full.\n");
            break;
        }
    }
}
