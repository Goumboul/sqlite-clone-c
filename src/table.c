#include <stdio.h>
#include <stdlib.h>

#include "table.h"

Table *table_create(void)
{
    Table *table = malloc(sizeof(Table));

    if (table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }

    return table;
}

void table_free(Table *table)
{
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        free(table->pages[i]);
    }
    free(table);
}

void *row_slot(Table *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;

    if (page_num >= TABLE_MAX_PAGES) {
        return NULL;
    }

    if (table->pages[page_num] == NULL) {
        table->pages[page_num] = malloc(PAGE_SIZE);
        if (table->pages[page_num] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }

    uint32_t byte_offset = (row_num % ROWS_PER_PAGE) * ROW_SIZE;
    return (char *)table->pages[page_num] + byte_offset;
}
