#include <stdio.h>
#include <stdlib.h>

#include "repl.h"

LineBuffer *line_buffer_create(void)
{
    LineBuffer *line = malloc(sizeof(LineBuffer));

    if (line == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    line->buffer = NULL;
    line->buffer_length = 0;
    line->input_length = 0;

    return line;
}

void line_buffer_free(LineBuffer *line)
{
    free(line->buffer);
    free(line);
}

void show_prompt(void)
{
    printf("db > ");
    fflush(stdout);
}

bool read_line(LineBuffer *line)
{
    ssize_t bytes_read = getline(&line->buffer, &line->buffer_length, stdin);

    if (bytes_read <= 0) {
        return false;
    }

    if (line->buffer[bytes_read - 1] == '\n') {
        /* Ignore trailing newline */
        line->input_length = bytes_read - 1;
        line->buffer[bytes_read - 1] = '\0';
    } else {
        /* Last line without a newline: keep every character */
        line->input_length = bytes_read;
    }

    return true;
}
