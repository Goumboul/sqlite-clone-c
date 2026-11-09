#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include <stdint.h>

#include "pager.h"

typedef struct {
    Pager *pager;
    uint32_t root_page_num;
} Table;

typedef struct {
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; /* true when positioned one past the last cell */
} Cursor;

Table *table_open(const char *filename);
void table_close(Table *table);
Cursor *table_start(Table *table);
Cursor *table_find(Table *table, uint32_t key);
void *cursor_value(Cursor *cursor);
void cursor_advance(Cursor *cursor);

#endif /* TABLE_H */
