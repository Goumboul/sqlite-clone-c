#ifndef ROW_H
#define ROW_H

#include <stdint.h>

#define COLUMN_USERNAME_SIZE (32)
#define COLUMN_EMAIL_SIZE (255)

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

#define ID_SIZE (sizeof(uint32_t))
#define USERNAME_SIZE (COLUMN_USERNAME_SIZE + 1)
#define EMAIL_SIZE (COLUMN_EMAIL_SIZE + 1)

#define ID_OFFSET (0)
#define USERNAME_OFFSET (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET (USERNAME_OFFSET + USERNAME_SIZE)

#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)

void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);
void print_row(Row *row);

#endif /* ROW_H */
