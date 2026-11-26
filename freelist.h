#ifndef FREELIST_H
#define FREELIST_H

#include <stddef.h>

typedef struct common_header {
    int size;                     // size of the memory block
    struct common_header *next;   // pointer to the next node of the freelist 
} common_header_t; // Header is common to both freelist and allocated block of memory




// Freelist related functions
common_header_t *get_list_head();
void init_free_list(void *mem, size_t mem_size);
int  fl_is_empty(void);
common_header_t *fl_first_fit(size_t num_bytes, common_header_t **prev_out);
void fl_split(size_t num_bytes, common_header_t *node, common_header_t *prev);
void fl_insert_freed_block(common_header_t *block);
void fl_dump(void);
size_t fl_free_mem_size(void);
size_t fl_req_mem(size_t payload);


#endif