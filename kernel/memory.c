#include <stdint.h>
#include <stddef.h>

#define HEAP_BASE 0x01000000u
#define HEAP_SIZE (8u * 1024u * 1024u)
#define ALIGN8(x) (((x) + 7u) & ~7u)

typedef struct block { uint32_t size; uint32_t free; struct block* next; } block_t;
static block_t* head;

void memory_init(void) {
    head = (block_t*)HEAP_BASE;
    head->size = HEAP_SIZE - sizeof(block_t);
    head->free = 1;
    head->next = 0;
}

void* kmalloc(uint32_t size) {
    if (!head || !size) return 0;
    size = ALIGN8(size);
    for (block_t* b = head; b; b = b->next) {
        if (b->free && b->size >= size) {
            if (b->size >= size + sizeof(block_t) + 8) {
                block_t* n = (block_t*)((uint8_t*)b + sizeof(block_t) + size);
                n->size = b->size - size - sizeof(block_t);
                n->free = 1; n->next = b->next;
                b->next = n; b->size = size;
            }
            b->free = 0;
            return (uint8_t*)b + sizeof(block_t);
        }
    }
    return 0;
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_t* b = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    b->free = 1;
    for (block_t* x = head; x && x->next; x = x->next) {
        if (x->free && x->next->free &&
            (uint8_t*)x + sizeof(block_t) + x->size == (uint8_t*)x->next) {
            x->size += sizeof(block_t) + x->next->size;
            x->next = x->next->next;
        }
    }
}
