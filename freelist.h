#ifndef FREELIST_H
#define FREELIST_H

#include <stddef.h>

typedef struct common_header {
    int size;                     // size of the memory block
    struct common_header *next;   // pointer to the next node of the freelist 
} common_header_t;

//Freelist related functions
void init_free_list(void *mem, size_t mem_size);
int  fl_is_empty(void);

#endif