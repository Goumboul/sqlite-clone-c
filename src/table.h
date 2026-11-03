#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

#include "row.h"

#define PAGE_SIZE (4096)
#define TABLE_MAX_PAGES (100)
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS (ROWS_PER_PAGE * TABLE_MAX_PAGES)

typedef struct {
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;

Table *table_create(void);
void table_free(Table *table);
void *row_slot(Table *table, uint32_t row_num);

#endif /* TABLE_H */
