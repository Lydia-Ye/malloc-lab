#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// structs node and header
typedef struct node {
  struct node* next;
} node_t;

typedef struct header {
  int magic_number;
  size_t size;
} header_t;

// Each of our free lists
node_t* free_lists[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))
size_t round_to_pow_of_two(size_t x);

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000
#define MAGIC_NUM 123456

// A utility logging function that definitely does not call malloc or free
void log_message(char* message);

/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {
  // The entire implementation of this function should be replaced.
  // This is a basic starting point so you can start with a working allocator, but this
  // implementation wastes an enormous amount of space.

  // if the requested memory is greater than 2048
  if (size > 2048) {
    size = ROUND_UP(size, PAGE_SIZE);
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    // Check for errors
    if (p == MAP_FAILED) {
      log_message("mmap failed! Giving up.\n");
      exit(2);
    }

    return p;
  }

  // Round the size up to the next multiple of the page size
  size = round_to_pow_of_two(size);

  // Find freelist index
  int index = log2(size) - 4;

  // if there is available memory in the appropiate list
  if (free_lists[index] != NULL) {
    node_t* memory = free_lists[index];
    free_lists[index] = memory->next;
    return (void*)memory;
  }

  //   Request memory from the operating system in page-sized chunks
  void* p = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // Check for errors
  if (p == MAP_FAILED) {
    log_message("mmap failed! Giving up.\n");
    exit(2);
  }

  // Track the end of allocated memory page
  void* page_end = p + PAGE_SIZE;

  // Initialize a new header for the current page
  header_t* header = (header_t*)p;
  header->magic_number = MAGIC_NUM;
  header->size = size;

  // Skip the header so that the pointer now pointing to the first free memory address
  p = p + size;

  // Link the allocated new memory to the free list
  free_lists[index] = (node_t*)p;

  node_t* current_node = free_lists[index];

  // seperate the page into size chunks
  while (p + size < page_end) {
    p = p + size;
    current_node->next = (node_t*)p;
    current_node = current_node->next;
  }

  current_node->next = NULL;  // end of the list

  // return the first chunk of memory after the header
  node_t* memory = free_lists[index];
  free_lists[index] = memory->next;

  return (void*)memory;
}

/**
 * Get the available size of an allocated object. This function should return the amount of space
 * that was actually allocated by malloc, not the amount that was requested.
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr) {
  // If ptr is NULL always return zero
  if (ptr == NULL) {
    return 0;
  }

  // Treat the freed pointer as an integer
  intptr_t free_address = (intptr_t)ptr;

  // Round down to the beginning of a page
  intptr_t page_start = free_address - (free_address % PAGE_SIZE);

  // Cast the page start address to a header struct
  header_t* header = (header_t*)page_start;

  // Check the magic number
  if (header->magic_number != MAGIC_NUM) {
    return 0;
  }

  return header->size;
}

/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
  // Don't free NULL!
  if (ptr == NULL) return;

  // get the size of the memory we want to free
  size_t size = xxmalloc_usable_size(ptr);

  // Make sure the memory is valid
  if (size == 0) return;

  // get the appropiate list from free_lists[]
  int index = log2(size) - 4;

  intptr_t free_address = (intptr_t)ptr;
  intptr_t object_start = free_address - (free_address % size);

  // Add the freed node to the strat of the freelist
  node_t* freed_node = (node_t*)object_start;
  freed_node->next = free_lists[index];
  free_lists[index] = freed_node;
}

/**
 * Print a message directly to standard error without invoking malloc or free.
 * \param message   A null-terminated string that contains the message to be printed
 */
void log_message(char* message) {
  // Get the message length
  size_t len = 0;
  while (message[len] != '\0') {
    len++;
  }

  // Write the message
  if (write(STDERR_FILENO, message, len) != len) {
    // Wrintite failed. Try to write an error message, then exit
    char fail_msg[] = "logging failed\n";
    write(STDERR_FILENO, fail_msg, sizeof(fail_msg));
    exit(2);
  }
}

/**
 * round x to the next power of two.
 * \param x  The number we will round to the next power of 2
 */
size_t round_to_pow_of_two(size_t x) {
  size_t current = MIN_MALLOC_SIZE;

  while (x > current) {
    current *= 2;
  }

  return current;
}
