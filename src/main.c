#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "query.h"
#include "repl.h"

int main(void)
{
    LineBuffer *line = line_buffer_create();

    while (true) {
        show_prompt();
        read_line(line);

        if (line->buffer[0] == '.') {
            if (handle_dot_command(line) == DOT_COMMAND_UNKNOWN) {
                printf("Command not found: '%s'.\n", line->buffer);
            }
            continue;
        }

        Query query;
        if (compile_query(line, &query) == COMPILE_UNKNOWN_QUERY) {
            printf("Unrecognized keyword at start of '%s'.\n", line->buffer);
            continue;
        }

        execute_query(&query);
        printf("Executed.\n");
    }
}
