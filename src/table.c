#include <stdio.h>
#include <stdlib.h>

#include "btree.h"
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
    table->root_page_num = 0;

    if (pager->num_pages == 0) {
        /* New database file: page 0 starts as an empty leaf node. */
        void *root_node = pager_get_page(pager, 0);
        initialize_leaf_node(root_node);
    }

    return table;
}

void table_close(Table *table)
{
    Pager *pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i] != NULL) {
            pager_flush(pager, i);
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
    void *root_node = pager_get_page(table->pager, table->root_page_num);

    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;
    cursor->end_of_table = (*leaf_node_num_cells(root_node) == 0);
    return cursor;
}

Cursor *table_end(Table *table)
{
    Cursor *cursor = cursor_create(table);
    void *root_node = pager_get_page(table->pager, table->root_page_num);

    cursor->page_num = table->root_page_num;
    cursor->cell_num = *leaf_node_num_cells(root_node);
    cursor->end_of_table = true;
    return cursor;
}

void *cursor_value(Cursor *cursor)
{
    void *page = pager_get_page(cursor->table->pager, cursor->page_num);
    return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(Cursor *cursor)
{
    void *node = pager_get_page(cursor->table->pager, cursor->page_num);

    cursor->cell_num += 1;
    if (cursor->cell_num >= *leaf_node_num_cells(node)) {
        cursor->end_of_table = true;
    }
}
