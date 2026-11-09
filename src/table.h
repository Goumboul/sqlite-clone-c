#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include <stdint.h>

#include "pager.h"
#include "row.h"

#define TABLE_MAX_ROWS (ROWS_PER_PAGE * TABLE_MAX_PAGES)

typedef struct {
    Pager *pager;
    uint32_t num_rows;
} Table;

typedef struct {
    Table *table;
    uint32_t row_num;
    bool end_of_table; /* true when positioned one past the last row */
} Cursor;

Table *table_open(const char *filename);
void table_close(Table *table);
Cursor *table_start(Table *table);
Cursor *table_end(Table *table);
void *cursor_value(Cursor *cursor);
void cursor_advance(Cursor *cursor);

#endif /* TABLE_H */
