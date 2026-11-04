#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pager.h"
#include "row.h"

Pager *pager_open(const char *filename)
{
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    if (file_length % ROW_SIZE != 0) {
        printf("Db file is not a whole number of rows. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    Pager *pager = malloc(sizeof(Pager));
    if (pager == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void *pager_get_page(Pager *pager, uint32_t page_num)
{
    if (page_num >= TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %u >= %d\n",
               page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL) {
        void *page = malloc(PAGE_SIZE);
        if (page == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        off_t page_start = (off_t)page_num * PAGE_DATA_SIZE;
        if (page_start < pager->file_length) {
            if (lseek(pager->file_descriptor, page_start, SEEK_SET) == -1) {
                perror("Error seeking");
                exit(EXIT_FAILURE);
            }
            if (read(pager->file_descriptor, page, PAGE_SIZE) == -1) {
                perror("Error reading file");
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
}

void pager_flush(Pager *pager, uint32_t page_num, uint32_t size)
{
    if (pager->pages[page_num] == NULL) {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }

    if (lseek(pager->file_descriptor, (off_t)page_num * PAGE_DATA_SIZE, SEEK_SET) == -1) {
        perror("Error seeking");
        exit(EXIT_FAILURE);
    }
    if (write(pager->file_descriptor, pager->pages[page_num], size) == -1) {
        perror("Error writing");
        exit(EXIT_FAILURE);
    }
}

void pager_close(Pager *pager)
{
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        free(pager->pages[i]);
    }
    if (close(pager->file_descriptor) == -1) {
        perror("Error closing db file");
        exit(EXIT_FAILURE);
    }
    free(pager);
}
