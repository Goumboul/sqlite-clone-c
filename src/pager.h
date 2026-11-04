#ifndef PAGER_H
#define PAGER_H

#include <stdint.h>
#include <sys/types.h>

#include "row.h"

#define PAGE_SIZE (4096)
#define TABLE_MAX_PAGES (100)
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)
/* Bytes a page occupies in the file: its rows, packed with no padding. */
#define PAGE_DATA_SIZE (ROWS_PER_PAGE * ROW_SIZE)

typedef struct {
    int file_descriptor;
    off_t file_length;
    void *pages[TABLE_MAX_PAGES];
} Pager;

Pager *pager_open(const char *filename);
void *pager_get_page(Pager *pager, uint32_t page_num);
void pager_flush(Pager *pager, uint32_t page_num, uint32_t size);
void pager_close(Pager *pager);

#endif /* PAGER_H */
