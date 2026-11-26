#include <stddef.h>
#include <sys/mman.h> 
#include "allocator.h"
#include "freelist.h"


// Total heap size reserved with mmap (example: 2 MB)


/// Used to initialise memory block
void *get_mem_block(void *addr, size_t mem_size) {
    void *h_ptr = mmap(
        addr,                   // starting address hint (NULL lets kernel choose)
        mem_size,               // number of bytes requested
        PROT_READ | PROT_WRITE, // memory should be readable and writable
        MAP_PRIVATE | MAP_ANONYMOUS, // private mapping, not backed by a file
        -1,                     // no file descriptor needed
        0                       // offset must be zero for MAP_ANONYMOUS
    );

    if (h_ptr == MAP_FAILED) {
        return NULL;            // return NULL if mmap fails
    }
    return h_ptr;               // otherwise return the allocated memory
}

//Used to prevent second memory block being assigned when all memory is allocated
static int initialised = 0; 

//Used to allocate memory
void *smalloc(size_t num_bytes){
    if (!initialised){
        void *mem = get_mem_block(NULL, MEM_SIZE);
        if (mem == NULL){
            return NULL;
        }
        init_free_list(mem, MEM_SIZE);
        initialised = 1;
    }

    common_header_t *prev;
    common_header_t *node = fl_first_fit(num_bytes, &prev);
    
    if (node == NULL){ //Failed to find contiguous memory block large enough for allocation 
        return NULL;
    }

    fl_split(num_bytes, node, prev);
    return (char*)node + sizeof(common_header_t);
}

//Used to free memory
void sfree(void *mem_ptr){
    if (mem_ptr == NULL){
        return;
    }
    char *header_addr = (char*)mem_ptr - sizeof(common_header_t);
    common_header_t *block = (common_header_t*)header_addr;
    fl_insert_freed_block(block);
}

void allocator_list_dump(void){
    fl_dump();
}


size_t allocator_free_mem_size(void){
    return fl_free_mem_size();
}


size_t allocator_req_mem(size_t payload){
    return fl_req_mem(payload);
}
