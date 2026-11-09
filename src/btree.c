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

static uint32_t *internal_node_num_keys(void *node)
{
    return (uint32_t *)((char *)node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

static uint32_t *internal_node_right_child(void *node)
{
    return (uint32_t *)((char *)node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

static uint32_t *internal_node_cell(void *node, uint32_t cell_num)
{
    return (uint32_t *)((char *)node + INTERNAL_NODE_HEADER_SIZE
                        + cell_num * INTERNAL_NODE_CELL_SIZE);
}

static uint32_t *internal_node_child(void *node, uint32_t child_num)
{
    uint32_t num_keys = *internal_node_num_keys(node);

    if (child_num > num_keys) {
        printf("Tried to access child_num %" PRIu32 " > num_keys %" PRIu32 "\n",
               child_num, num_keys);
        exit(EXIT_FAILURE);
    }
    if (child_num == num_keys) {
        return internal_node_right_child(node);
    }
    return internal_node_cell(node, child_num);
}

static uint32_t *internal_node_key(void *node, uint32_t key_num)
{
    return (uint32_t *)((char *)internal_node_cell(node, key_num)
                        + INTERNAL_NODE_CHILD_SIZE);
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

static bool is_node_root(void *node)
{
    uint8_t value = *((uint8_t *)((char *)node + IS_ROOT_OFFSET));
    return (bool)value;
}

void set_node_root(void *node, bool is_root)
{
    uint8_t value = is_root;
    *((uint8_t *)((char *)node + IS_ROOT_OFFSET)) = value;
}

void initialize_leaf_node(void *node)
{
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
}

static void initialize_internal_node(void *node)
{
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
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

static uint32_t get_unused_page_num(Pager *pager)
{
    /* Pages are only appended for now, so the next free page is num_pages. */
    return pager->num_pages;
}

static uint32_t get_node_max_key(void *node)
{
    if (get_node_type(node) == NODE_INTERNAL) {
        return *internal_node_key(node, *internal_node_num_keys(node) - 1);
    }
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
}

/*
 * The root page is kept at page 0. Its current contents move to a new left
 * child, and the root becomes an internal node with two children.
 */
static void create_new_root(Table *table, uint32_t right_child_page_num)
{
    void *root = pager_get_page(table->pager, table->root_page_num);
    uint32_t left_child_page_num = get_unused_page_num(table->pager);
    void *left_child = pager_get_page(table->pager, left_child_page_num);

    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    *internal_node_key(root, 0) = get_node_max_key(left_child);
    *internal_node_right_child(root) = right_child_page_num;
}

static void leaf_node_split_and_insert(Cursor *cursor, uint32_t key,
                                       Row *value)
{
    void *old_node = pager_get_page(cursor->table->pager, cursor->page_num);
    uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
    void *new_node = pager_get_page(cursor->table->pager, new_page_num);
    initialize_leaf_node(new_node);

    /*
     * Distribute the existing cells and the new one between the two nodes.
     * Walk from the right so that no cell is overwritten before it is moved.
     */
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void *destination_node = (i >= (int32_t)LEAF_NODE_LEFT_SPLIT_COUNT)
                                     ? new_node
                                     : old_node;
        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;

        if (i == (int32_t)cursor->cell_num) {
            *leaf_node_key(destination_node, index_within_node) = key;
            serialize_row(value,
                          leaf_node_value(destination_node, index_within_node));
        } else if (i > (int32_t)cursor->cell_num) {
            memcpy(leaf_node_cell(destination_node, index_within_node),
                   leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(leaf_node_cell(destination_node, index_within_node),
                   leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *leaf_node_num_cells(old_node) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *leaf_node_num_cells(new_node) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) {
        create_new_root(cursor->table, new_page_num);
    } else {
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
    }
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
    void *node = pager_get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        leaf_node_split_and_insert(cursor, key, value);
        return;
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

static void indent(uint32_t level)
{
    for (uint32_t i = 0; i < level; i++) {
        printf("  ");
    }
}

void print_tree(Pager *pager, uint32_t page_num, uint32_t indentation_level)
{
    void *node = pager_get_page(pager, page_num);
    uint32_t num_keys;
    uint32_t child;

    switch (get_node_type(node)) {
    case NODE_LEAF:
        num_keys = *leaf_node_num_cells(node);
        indent(indentation_level);
        printf("- leaf (size %" PRIu32 ")\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) {
            indent(indentation_level + 1);
            printf("- %" PRIu32 "\n", *leaf_node_key(node, i));
        }
        break;
    case NODE_INTERNAL:
        num_keys = *internal_node_num_keys(node);
        indent(indentation_level);
        printf("- internal (size %" PRIu32 ")\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) {
            child = *internal_node_child(node, i);
            print_tree(pager, child, indentation_level + 1);

            indent(indentation_level + 1);
            printf("- key %" PRIu32 "\n", *internal_node_key(node, i));
        }
        child = *internal_node_right_child(node);
        print_tree(pager, child, indentation_level + 1);
        break;
    }
}
