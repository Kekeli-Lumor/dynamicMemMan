#include <stddef.h>
#include <stdio.h>
#include "freelist.h"



static common_header_t *list_head = NULL;
void init_free_list(void *mem, size_t mem_size) {
    // Initialize the freelist (list_head is assumed to be defined in this scope)
    list_head = (common_header_t *)mem;
    list_head->size = (int)(mem_size - sizeof(common_header_t));
    list_head->next = NULL;
}

int fl_is_empty(void) { 
    return list_head == NULL;
}

common_header_t *fl_first_fit(size_t num_bytes, common_header_t **prev_out) {
    common_header_t *prev = NULL;
    common_header_t *node = list_head;

    while (node != NULL) {
        if ((size_t)node->size >= num_bytes) { // fits (split-or-not is Step 3's job)
            *prev_out = prev;
            return node;
        }
        prev = node;
        node = node->next;
    }
    return NULL; // nothing fits
}

void fl_split(size_t num_bytes, common_header_t *node, common_header_t *prev){
    int old_size = node->size;
    common_header_t *old_next = node->next;
    common_header_t *new_fl_entry;

    if ((size_t)old_size > num_bytes + sizeof(common_header_t)){
        //Assign remaining free memory block after allocated section
        char *remaining_mem_addr =(char*)node + sizeof(common_header_t) + num_bytes;
        common_header_t *remaining_mem = (common_header_t *)remaining_mem_addr;
        
        remaining_mem->size = old_size - (int)num_bytes - (int)sizeof(common_header_t);
        remaining_mem->next = old_next;

        node->size = (int)num_bytes;
        new_fl_entry = remaining_mem;
    } 
    else{
        //Assign whole memory block if there is no remaining memory after allocation 
        new_fl_entry = old_next;
    }
    
    //Link whatever pointed to node to the new free list entry 
    if (prev == NULL){
        list_head = new_fl_entry;
    }
    else{
        prev->next = new_fl_entry;
    }

    node->next = NULL;
}

void fl_insert_freed_block(common_header_t *block) {
    block->next = list_head;
    list_head = block;
}

//Prints Freelist as [size] -> ...
void fl_dump(void){
    common_header_t *node = list_head;
    while (node != NULL){
        printf("[%d]", node->size);
        if (node->next != NULL){
            printf(" -> ");
        }
        node = node->next;
    }
    printf("\n");
}

//Returns total free payload bytes
size_t fl_free_mem_size(void){
    size_t total = 0;
    common_header_t *node = list_head;
    while (node != NULL){
        total += (size_t)node->size;
        node = node->next;
    }
    return total;
}

//Returns total bytes required for a given payload
size_t fl_req_mem(size_t payload){
    return payload + sizeof(common_header_t);
}