#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#define MEM_SIZE (10 * 1024 * 1024) //10MB

void *smalloc(size_t num_bytes);
void sfree(void *mem_ptr);

//Allocator helper functions
void allocator_list_dump(void); 
size_t allocator_free_mem_size(void); 
size_t allocator_req_mem(size_t payload);
void allocator_stats(size_t *N, size_t *F, size_t *L); 

#endif