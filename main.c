#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int PAGE_SIZE = 4096;
const int MAX_PAGE = 100;
const int HEADER_PAGE = 0;
const int FIRST_FREE_PAGE = 2;

/**
 * The page manager struct
 * It contains the free pages, the max page and the next free page
 * The free pages is an array of integers that contains the free pages
 * The max page is the maximum number of pages in the file
 * The next free page is the index of the next free page in the free pages array
 * The first free page is 2 because the first page is for the header
 *
 * The page manager is used to manage the pages in the file
 * It is used to allocate and release pages
 */
struct PageManager {
  int *free_pages;
  int max_page;
  int next_free_page;
};

/**
 * The page struct
 * It contains the page number and the data
 */
struct Page {
  int id;          // the page number
  char data[4096]; // the data in the page
};

/**
 * The DAL (Data Access Layer) struct
 * It contains the file pointer, the page size and the page manager
 * The page manager is used to manage the pages in the file
 */
struct Dal {
  FILE *fp;
  int page_size;
  struct PageManager *pm;
};

/**
 * Open the file
 */
void dal_open(struct Dal *dal, const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (fp != NULL) {
    dal->fp = fp;
  }
}

// Close the file
void dal_close(struct Dal *dal) { fclose(dal->fp); }

/**
 * Allocate an empty page
 */
struct Page *allocate_empty_page(struct Dal *dal) {
  struct Page *page = malloc(sizeof(struct Page));
  page->id = -1;
  page->data[dal->page_size - 1] = '\0'; // null-terminate the data
  return page;
}

/**
 * Read a page from the file
 */
struct Page *read_page(struct Dal *dal, int page_number) {
  struct Page *page = allocate_empty_page(dal);
  fseek(dal->fp, page_number * dal->page_size, SEEK_SET);
  fread(page->data, dal->page_size, 1, dal->fp);
  return page;
}

/**
 * Write a page to the file
 */
void write_page(struct Page *page, struct Dal *dal) {
  fseek(dal->fp, page->id * dal->page_size, SEEK_SET);
  fwrite(page->data, dal->page_size, 1, dal->fp);
}
/**
 * Initialize the page manager
 */
struct PageManager *initialize_page_manager(int max_page) {
  struct PageManager *pm = malloc(sizeof(struct PageManager));
  if (pm != NULL) {
    pm->max_page = max_page;
    // the first free page is 2
    // first page is for the header
    pm->next_free_page = FIRST_FREE_PAGE;
    pm->free_pages = malloc(sizeof(int) * max_page);
    if (pm->free_pages == NULL) {
      free(pm);
      return NULL;
    }
    // initialize all pages as free
    for (int i = 0; i < max_page; i++) {
      pm->free_pages[i] = i;
    }
  }
  return pm;
}

/**
 * Release a page
 */
void release_page(struct PageManager *pm, int page_num) {
  if (pm != NULL && pm->free_pages != NULL &&
      pm->next_free_page < pm->max_page) {
    pm->free_pages[pm->next_free_page] = page_num;
    pm->next_free_page++;
  }
}

/**
 * Get the next free page
 */
int get_next_free_page(struct PageManager *pm) {
  if (pm != NULL && pm->free_pages != NULL && pm->next_free_page > 0) {
    pm->next_free_page--;
    return pm->free_pages[pm->next_free_page];
  }
  return -1;
}

/**
 * Create a new DAL
 */
struct Dal *create_dal(const char *filename, int page_size, int max_page) {
  struct Dal *dal = malloc(sizeof(struct Dal));
  struct PageManager *pm = initialize_page_manager(max_page);
  if (pm == NULL) {
    free(dal);
    return NULL;
  }
  dal->page_size = page_size;
  dal->pm = pm;
  return dal;
}

/**
 * Free the memory allocated for the DAL
 */
void free_dal(struct Dal *dal) {
  free(dal->pm->free_pages);
  dal->pm->free_pages = NULL;
  free(dal->pm);
  dal->pm = NULL;
  free(dal);
  dal = NULL;
}

int main() {
  struct Dal *dal = create_dal("test.db", PAGE_SIZE, MAX_PAGE);
  struct Page *page = allocate_empty_page(dal);
  page->id = get_next_free_page(dal->pm);
  strcpy(page->data, "data");
  dal_open(dal, "test.db");
  write_page(page, dal);
  dal_close(dal);

  free(page);
  page = NULL;
  free_dal(dal);

  return EXIT_SUCCESS;
}
