#include <stdio.h>
#include <stdlib.h>

#include "table.h"

Table *table_open(const char *filename)
{
    Pager *pager = pager_open(filename);
    Table *table = malloc(sizeof(Table));

    if (table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    table->pager = pager;
    table->num_rows = pager->file_length / ROW_SIZE;

    return table;
}

void table_close(Table *table)
{
    Pager *pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; i++) {
        if (pager->pages[i] != NULL) {
            pager_flush(pager, i, PAGE_DATA_SIZE);
        }
    }

    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != NULL) {
            pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
        }
    }

    pager_close(pager);
    free(table);
}

static Cursor *cursor_create(Table *table)
{
    Cursor *cursor = malloc(sizeof(Cursor));

    if (cursor == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    cursor->table = table;
    return cursor;
}

Cursor *table_start(Table *table)
{
    Cursor *cursor = cursor_create(table);

    cursor->row_num = 0;
    cursor->end_of_table = (table->num_rows == 0);
    return cursor;
}

Cursor *table_end(Table *table)
{
    Cursor *cursor = cursor_create(table);

    cursor->row_num = table->num_rows;
    cursor->end_of_table = true;
    return cursor;
}

void *cursor_value(Cursor *cursor)
{
    uint32_t row_num = cursor->row_num;
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = pager_get_page(cursor->table->pager, page_num);
    uint32_t byte_offset = (row_num % ROWS_PER_PAGE) * ROW_SIZE;
    return (char *)page + byte_offset;
}

void cursor_advance(Cursor *cursor)
{
    cursor->row_num += 1;
    if (cursor->row_num >= cursor->table->num_rows) {
        cursor->end_of_table = true;
    }
}
