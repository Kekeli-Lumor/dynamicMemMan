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

//Searches for the first memory block large enough to accommodate the request
common_header_t *fl_first_fit(size_t num_bytes, common_header_t **prev_out) {
    common_header_t *prev = NULL;
    common_header_t *node = list_head;

    while (node != NULL) {
        if ((size_t)node->size >= num_bytes) { //Finds contiguous free memory slot available that's large enough
            *prev_out = prev;
            return node;
        }
        prev = node;
        node = node->next;
    }
    return NULL; //No contiguous free memory slot available that's large enough
}

//Searches for the smallest memory block large enough to accommodate the request
common_header_t *fl_best_fit(size_t num_bytes, common_header_t **prev_out) {
    common_header_t *prev = NULL;
    common_header_t *node = list_head;
    common_header_t *best_node = NULL;
    *prev_out = NULL;

    while (node != NULL) {
        if ((size_t)node->size >= num_bytes) {
            if (best_node == NULL || node->size < best_node->size){
                best_node = node;
                *prev_out = prev;
                if ((size_t)best_node->size == num_bytes){
                    return best_node; //Returns first memory slot of exact right size
                }
            }
        }
        prev = node;
        node = node->next;
    }
    return best_node;
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

//Inserts freed block at the head of the freelist
void fl_head_insert(common_header_t *block) {
    block->next = list_head;
    list_head = block;
}

//Inserts a freed block into the freelist in address order
//Returns prev so that blocks can be merged if conditions are met
common_header_t *fl_ordered_insert(common_header_t *block) {
    common_header_t *prev = NULL;
    common_header_t *node = list_head;

    //Finds first node after freed block
    while (node != NULL && node < block){
        prev = node;
        node = node->next;
    }

    block->next = node; //Freed block points to the node after it

    //If no free block comes before it, it becomes the new head
    //otherwise the previous node points to newly freed block
    if (prev == NULL){ 
        list_head = block;
    }
    else{
        prev->next = block; 
    }
    return prev;
}

void fl_merge(common_header_t *prev, common_header_t *block){
    if (block->next != NULL){
        char *block_end = (char*)block + sizeof(common_header_t) + block->size;
        if (block_end == (char*)block->next){
            block->size += (int)sizeof(common_header_t) + (block->next)->size;
            block->next = (block->next)->next;
        }    
    }

    if (prev != NULL){
        char *prev_end = (char*)prev + sizeof(common_header_t) + prev->size;
        if (prev_end == (char*)block){
            prev->size += (int)sizeof(common_header_t) + block->size;
            prev->next = block->next;
        }    
    }
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

void fl_stats(size_t *N, size_t *F, size_t *L){
    common_header_t *node = list_head;
    size_t num_free = 0, largest = 0, total = 0;
    while (node != NULL){
        total += (size_t)node->size;
        num_free++;
        if ((size_t)node->size > largest){
            largest = (size_t)node->size;
        }
        node = node->next;
    }
    *N = num_free; //N: number of free blocks  
    *F = total; //F: amount of free memory (in bytes)
    *L = largest; //L: size of the largest free block (in bytes)
}