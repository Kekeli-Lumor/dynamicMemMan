#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#define MEM_SIZE 1024

void *smalloc(size_t num_bytes);
void sfree(void *mem_ptr);
void allocator_list_dump(void); 
size_t allocator_free_mem_size(void); 
size_t allocator_req_mem(size_t payload);



#endif