#include <stddef.h>
#include <sys/mman.h>
#include "allocator.h"
#include "freelist.h"


/// Used to initialise memory block
static void *get_mem_block(void *addr, size_t mem_size) {
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


//Head of the freelist for a single selected heap
static common_header_t *heap_head = NULL;


//Used to allocate memory
void *smalloc(size_t num_bytes){
    if (!initialised){
        void *mem = get_mem_block(NULL, MEM_SIZE);
        if (mem == NULL){
            return NULL;
        }
        init_free_list(&heap_head, mem, MEM_SIZE);
        initialised = 1;
    }


   
    common_header_t *prev;
   
    //Use First Fit Algo
    // common_header_t *node = fl_first_fit(&heap_head, num_bytes, &prev);


    //Use Best Fit Algo
    common_header_t *node = fl_best_fit(&heap_head, num_bytes, &prev);
   
   
    if (node == NULL){ //Failed to find contiguous memory block large enough for allocation
        return NULL;
    }


    fl_split(&heap_head, num_bytes, node, prev);
    return (char*)node + sizeof(common_header_t);
}


static void allocator_merge(common_header_t *prev, common_header_t *block) {
    fl_merge(prev, block);
}


//Used to free memory
void sfree(void *mem_ptr){
    if (mem_ptr == NULL){
        return;
    }
    char *header_addr = (char*)mem_ptr - sizeof(common_header_t);
    common_header_t *block = (common_header_t*)header_addr;
   
    //Use Head Insert
    // fl_head_insert(&heap_head, block);


    //Use Ordered Insert + Merge Algo
    common_header_t *prev = fl_ordered_insert(&heap_head, block);
    allocator_merge(prev, block);
}


void allocator_list_dump(void){
    fl_dump(&heap_head);
}




size_t allocator_free_mem_size(void){
    return fl_free_mem_size(&heap_head);
}




size_t allocator_req_mem(size_t payload){
    return fl_req_mem(payload);
}


void allocator_stats(size_t *N, size_t *F, size_t *L){
    fl_stats(&heap_head, N, F, L);
}
