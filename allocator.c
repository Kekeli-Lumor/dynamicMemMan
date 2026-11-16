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


