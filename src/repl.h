#ifndef REPL_H
#define REPL_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} LineBuffer;

LineBuffer *line_buffer_create(void);
void line_buffer_free(LineBuffer *line);
void show_prompt(void);
bool read_line(LineBuffer *line);

#endif /* REPL_H */
