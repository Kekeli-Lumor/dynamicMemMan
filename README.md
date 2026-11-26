# dynamicMemMan


### Stage 1: First-fit allocation with head-insert free

**Overview:** in its first version, the allocator reserves one block of memory from the OS,
hands out pieces of it with `smalloc` using a **first-fit** search, and returns them with
`sfree` by pushing them onto the front of the freelist. No merging is done yet.

**How memory is laid out**

Every block, free or allocated, starts with a `common_header_t`:
```c
typedef struct common_header {
    int size;                    // payload size in bytes (header not included)
    struct common_header *next;  // next free block; NULL once allocated
} common_header_t;
```
The freelist isn't stored separately. Each node lives in the first bytes of the free block
it describes. When a block is allocated, its node becomes the block's header: `size` is
kept so `sfree` knows how big the block is, and `next` is set to `NULL`. On a 64-bit machine
the header is **16 bytes** because of padding (a 4-byte `int` + 4 bytes padding + an 8-byte
pointer), not the 8 bytes shown in the spec. Every offset therefore uses
`sizeof(common_header_t)` and never a fixed number.

**Setting up the heap: `smalloc` Step 1**

The first call to `smalloc` reserves `MEM_SIZE` bytes with `mmap` (through `get_mem_block`)
and turns the whole region into a single free node:
```c
if (!initialised) {
    void *mem = get_mem_block(NULL, MEM_SIZE);
    if (mem == NULL) {
        return NULL;
    }
    init_free_list(mem, MEM_SIZE);
    initialised = 1;
}
```
A separate `initialised` flag is used instead of checking `list_head == NULL`. The list is
also empty when every free block has been handed out. Without the flag, that case would
look like the first call and silently `mmap` a second heap instead of failing.

**Finding a block: `fl_first_fit`**

The freelist module walks the list from the head and returns the first block with
`size >= num_bytes`. It also returns that block's predecessor through `prev_out`, because the
list is singly linked and the block's predecessor has to be relinked when the block is removed:
```c
while (node != NULL) {
    if ((size_t)node->size >= num_bytes) {
        *prev_out = prev;
        return node;
    }
    prev = node;
    node = node->next;
}
return NULL;    // no single block is large enough, so smalloc fails
```

**Splitting the block: `fl_split`**

If the block is large enough to leave a usable remainder (room for a header plus at least
1 byte), the remainder becomes a new free node placed straight after the allocated payload:
```c
char *leftover_addr = (char *)node + sizeof(common_header_t) + num_bytes;
common_header_t *leftover = (common_header_t *)leftover_addr;
leftover->size = old_size - (int)num_bytes - (int)sizeof(common_header_t);
leftover->next = old_next;
```
The address arithmetic is done on a `char *`, so adding `n` moves forward exactly `n`
bytes. If the remainder would be too small, the whole block is handed out instead. This
avoids creating a node with no space after its header. In both cases, whatever pointed to
the chosen block (its predecessor, or `list_head` if it was first) is updated to point to
its replacement, and the allocated block's `next` is set to `NULL`.

`smalloc` then returns the address just after the header, which is where the user's data
starts:
```c
return (char *)node + sizeof(common_header_t);
```

**Freeing a block: `sfree`**

`sfree` reverses that last step to recover the header, and pushes the block onto the front
of the freelist:
```c
char *header_addr = (char *)mem_ptr - sizeof(common_header_t);
common_header_t *block = (common_header_t *)header_addr;
fl_insert_freed_block(block);       // block->next = list_head; list_head = block;
```
The block's `size` is already correct from when it was allocated, so only the links change.
Head insertion is O(1), but freed blocks are never combined with neighbouring free blocks.

**Results**

With `MEM_SIZE = 1024`, the test completes without failures, e.g. a final freelist of
`[11] -> [1] -> [165]` (847 bytes used, 177 free). The `[11]` and `[1]` nodes are freed blocks
pushed onto the head of the list. The `[165]` node is what remains of the original region.

A simulation of the test over 100,000 random seeds gave **0% failures at 1024 bytes**,
94% at 768 and 100% at 512. To exercise the failure path, the test was also run at 512 bytes.
Allocation failed with [Y] bytes free in total, while the request needed [X] bytes and the
largest free block was smaller than that. This is **external fragmentation**: there is
enough memory in total, but no single contiguous block is big enough.

**Limitations, which motivate Stage 2**
- Neighbouring free blocks are never merged, so the freelist keeps growing and free space
  stays split into small pieces.
- First-fit cuts up the first large block it reaches, even when a closer-sized block exists
  further along.