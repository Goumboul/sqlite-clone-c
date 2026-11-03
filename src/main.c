#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repl.h"

int main(void)
{
    LineBuffer *line = line_buffer_create();

    while (true) {
        show_prompt();
        read_line(line);

        if (strcmp(line->buffer, ".exit") == 0) {
            line_buffer_free(line);
            exit(EXIT_SUCCESS);
        }
        printf("Command not found: '%s'.\n", line->buffer);
    }
}
