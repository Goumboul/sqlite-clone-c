#ifndef LINE_BUFFER_H
    #define LINE_BUFFER_H

    #include <stddef.h>
    #include <sys/types.h>

    typedef struct {
        char *buffer;
        size_t buffer_length;
        ssize_t input_length;
    } LineBuffer;

#endif
