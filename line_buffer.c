#include <stdio.h>
#include <stdlib.h>

#include "line_buffer.h"

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

void read_line(LineBuffer *line)
{
    line->input_length = getline(&line->buffer, &line->buffer_length, stdin);

    if (line->input_length <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    line->buffer[line->input_length - 1] = '\0';
}
