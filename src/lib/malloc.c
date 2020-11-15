/*
 * File: malloc.c
 * --------------
 * This is a bump-header heap allocator, capable of recycling memory.
 * 
 */

#include "malloc.h"
#include "malloc_internal.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"

extern int __bss_end__;

/*
 * The pool of memory available for the heap starts at the upper end of the 
 * data section and can extend from there up to the lower end of the stack.
 * It uses symbol __bss_end__ from memmap to locate data end
 * and calculates stack end assuming a 16MB stack.
 *
 * Global variables for the bump allocator:
 *
 * `heap_start`  location where heap segment starts
 * `heap_end`    location at end of in-use portion of heap segment 
 */

// Initial heap segment starts at bss_end and is empty
static void *heap_start = &__bss_end__;
static void *heap_end = &__bss_end__;


/*
 * Definitions, Constants, and Macros
 */

#define STACK_START 0x8000000
#define STACK_SIZE  0x1000000
#define STACK_END ((char *)STACK_START - STACK_SIZE)
#define STRUCT_SIZE 8

// Simple macro to round up x to multiple of n - works onlky for power of 2
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1))) 

/*
 * Block Header Sturcture
 * Size: 8 Bytes
 * Contains:
 *      Payload Size (4 byte unsigned int)
 *      Status omf block (4 byte signed int)
 */

struct header {
    size_t payload_size;
    int status;
};

enum {
    FREE = 0,
    IN_USE = 1
};

typedef struct header hdr_t;


/* Program */     

// helper function to combine adjacent empty blocks
static void coalesce_block(hdr_t *hdr1) {
    hdr_t *next_block = hdr1 + (hdr1->payload_size / 8) + 1;
    unsigned int size = hdr1->payload_size;
    while(((void *)next_block < heap_end) && (next_block->status != IN_USE)) {
        size += next_block->payload_size;
        next_block += ((next_block->payload_size / 8) + 1);
    }
    hdr1->payload_size = size;
}

// helper function to search heap for free space
static void *search_heap(int nbytes) {
    hdr_t *new_block = (hdr_t *)heap_start;
    while((void *)new_block <= (void *)heap_end) {
        if((new_block->payload_size >= nbytes) && (new_block->status != IN_USE)) {
            return new_block;
        } else {
            new_block += ((new_block->payload_size / 8) + 1);
        }
    }
    return NULL; 
}

void *sbrk(int nbytes)
{
    void *prev_end = heap_end;
    if ((char *)prev_end + nbytes > STACK_END) {
        return NULL;
    } else {
        heap_end = (char *)prev_end + nbytes;
        return prev_end;
    }
}

void *malloc (size_t nbytes)
{ 
    if(nbytes == 0) return NULL;
    nbytes = roundup(nbytes, 8);
    hdr_t *search_result = search_heap(nbytes);
    if(heap_start != heap_end && search_result != NULL) { // if not first block, search for space in heap 
        search_result->payload_size = nbytes;
        search_result->status = IN_USE;
        return search_result + 1;
    } else { // add to end of heap
        search_result = sbrk(STRUCT_SIZE); // new header (8 bytes) created at end of heap
        search_result->payload_size = nbytes; // size set to rounded nbytes
        search_result->status = IN_USE; // status is "in use"
        return sbrk(nbytes);
    }
}

void free (void *ptr)
{
    if(ptr != NULL) {
        hdr_t *free_block = (hdr_t *)ptr - 1;
        free_block->status = FREE;
        coalesce_block(free_block);
    }
}

void *realloc (void *orig_ptr, size_t new_size)
{
    if(orig_ptr == NULL) {return malloc(new_size);};
    hdr_t *base_block = orig_ptr;
    free(orig_ptr);
    base_block--;

    void *orig_start = heap_start;
    heap_start = base_block;
    void *new_ptr = malloc(new_size);
    
    // if full, return null, else copy previous data to new ptr
    if (!new_ptr) return NULL;
    memcpy(new_ptr, orig_ptr, new_size);
    heap_start = orig_start; 
    return new_ptr;
}

void heap_dump (const char *label)
{
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", heap_start, heap_end);
   
    hdr_t *header = (hdr_t *)heap_start;
    while((void *)header < (void *)heap_end) {
        printf("Block at: %p\n", header);
        printf("Size: %d at %p\n", header->payload_size, &header->payload_size);
        printf("Status: %d at %p\n", header->status, &header->status);
        header += ((header->payload_size / 8) + 1);
    }
   
    printf("----------  END DUMP (%s) ----------\n", label);
}

void memory_report (void)
{
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Memory Report         \n");
    printf(  "=============================================\n");
    // TODO: fill in for extension
}
