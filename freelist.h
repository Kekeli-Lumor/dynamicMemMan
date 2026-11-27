#ifndef FREELIST_H
#define FREELIST_H


#include <stddef.h>


typedef struct common_header {
    int size; //size of the memory block
    struct common_header *next; //pointer to the next node of the freelist
} common_header_t; //Header is common to both freelist and allocated block of memory






//Initialisation functions
void init_free_list(common_header_t **head, void *mem, size_t mem_size);


//Memory allocation functions
common_header_t *fl_first_fit(common_header_t **head, size_t num_bytes, common_header_t **prev_out);
common_header_t *fl_best_fit(common_header_t **head, size_t num_bytes, common_header_t **prev_out);
void fl_split(common_header_t **head, size_t num_bytes, common_header_t *node, common_header_t *prev);


//Memory deallocation functions
void fl_head_insert(common_header_t **head, common_header_t *block);
common_header_t *fl_ordered_insert(common_header_t **head, common_header_t *block);
void fl_merge(common_header_t *prev, common_header_t *block);


//Stats related functions
void fl_dump(common_header_t **head);
size_t fl_free_mem_size(common_header_t **head);
size_t fl_req_mem(size_t payload);
void fl_stats(common_header_t **head, size_t *N, size_t *F, size_t *L);


#endif
