#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

#include "pager.h"
#include "row.h"

#define TABLE_MAX_ROWS (ROWS_PER_PAGE * TABLE_MAX_PAGES)

typedef struct {
    Pager *pager;
    uint32_t num_rows;
} Table;

Table *table_open(const char *filename);
void table_close(Table *table);
void *row_slot(Table *table, uint32_t row_num);

#endif /* TABLE_H */
