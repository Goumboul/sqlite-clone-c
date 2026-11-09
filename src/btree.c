#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

uint32_t *leaf_node_num_cells(void *node)
{
    return (uint32_t *)((char *)node + LEAF_NODE_NUM_CELLS_OFFSET);
}

void *leaf_node_cell(void *node, uint32_t cell_num)
{
    return (char *)node + LEAF_NODE_HEADER_SIZE
           + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t *leaf_node_key(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num);
}

void *leaf_node_value(void *node, uint32_t cell_num)
{
    return (char *)leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

NodeType get_node_type(void *node)
{
    uint8_t value = *((uint8_t *)((char *)node + NODE_TYPE_OFFSET));
    return (NodeType)value;
}

void set_node_type(void *node, NodeType type)
{
    uint8_t value = type;
    *((uint8_t *)((char *)node + NODE_TYPE_OFFSET)) = value;
}

void initialize_leaf_node(void *node)
{
    set_node_type(node, NODE_LEAF);
    *leaf_node_num_cells(node) = 0;
}

/*
 * Return the position of the key in the leaf. If the key is not present,
 * return the position where it should be inserted.
 */
Cursor *leaf_node_find(Table *table, uint32_t page_num, uint32_t key)
{
    void *node = pager_get_page(table->pager, page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    Cursor *cursor = malloc(sizeof(Cursor));
    if (cursor == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    cursor->table = table;
    cursor->page_num = page_num;

    /* Binary search over the cells, which are kept sorted by key. */
    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;
    while (one_past_max_index != min_index) {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);
        if (key == key_at_index) {
            min_index = index;
            break;
        }
        if (key < key_at_index) {
            one_past_max_index = index;
        } else {
            min_index = index + 1;
        }
    }

    cursor->cell_num = min_index;
    cursor->end_of_table = (min_index >= num_cells);
    return cursor;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
    void *node = pager_get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        /* Splitting a leaf node is not implemented yet. */
        printf("Need to implement splitting a leaf node.\n");
        exit(EXIT_FAILURE);
    }

    if (cursor->cell_num < num_cells) {
        /* Make room for the new cell. */
        for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    *leaf_node_num_cells(node) += 1;
    *leaf_node_key(node, cursor->cell_num) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

void print_constants(void)
{
    printf("ROW_SIZE: %d\n", (int)ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", (int)COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", (int)LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", (int)LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", (int)LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", (int)LEAF_NODE_MAX_CELLS);
}

void print_leaf_node(void *node)
{
    uint32_t num_cells = *leaf_node_num_cells(node);

    printf("leaf (size %" PRIu32 ")\n", num_cells);
    for (uint32_t i = 0; i < num_cells; i++) {
        printf("  - %" PRIu32 " : %" PRIu32 "\n", i, *leaf_node_key(node, i));
    }
}
