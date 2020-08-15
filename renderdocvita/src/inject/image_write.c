#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image_write.h"

#define BUFFER_SIZE 2 * 1024 * 1024
static uint8_t buffer[BUFFER_SIZE];
static size_t allocatedSize;

struct header_t
{
    size_t size;
    unsigned is_free;
    struct header_t *next;
};

static struct header_t *head, *tail;

static struct header_t *get_free_block(size_t size)
{
    struct header_t *curr = head;
    while(curr) {
        if (curr->is_free && curr->size >= size)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

void* custom_malloc(size_t size) {
    size_t total_size;
    void *block;
    struct header_t *header;
    
    if (!size)
        return NULL;
    
    header = get_free_block(size);
    
    if (header) {
        header->is_free = 0;
        return (void*)(header + 1);
    }

    total_size = sizeof(struct header_t) + size;
    block = &buffer[allocatedSize];
    allocatedSize += total_size;
    if (block == (void*) -1) {
        return NULL;
    }
    header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;
    if (!head)
        head = header;
    if (tail)
        tail->next = header;
    tail = header;
    return (void*)(header + 1);
}


void custom_free(void* block) {   
    struct header_t *header, *tmp;
    void *programbreak;
    
    if (!block)
        return;
        
    /* get pointer to meta-data header */
    header = (struct header_t*)block - 1;
    
    /* get current top of heap */
    programbreak = &buffer[allocatedSize];
    
    /* the block that we want to remove is the last block in the list */
    if ((char*)block + header->size == programbreak) {
        /* If this is the only block left in linked list then remove pointer */
        if (head == tail) {
            head = tail = NULL;
        }
        
        /* Else remove last block from list */
        else {
            tmp = head;
            while (tmp) {
                if(tmp->next == tail) {
                    tmp->next = NULL;
                    tail = tmp;
                }
                tmp = tmp->next;
            }
        }
        
        /* Using -ve number to reduce heap size */
        allocatedSize -= sizeof(struct header_t) - header->size;
        return;
    }
    header->is_free = 1;
}

void* custom_realloc(void* block, size_t size) {
    struct header_t *header;
    void *ret;
    
    /* If block don't exist, then just malloc and return */
    if (!block || !size)
        return custom_malloc(size);
    
    /* Allocate new block */
    header = (struct header_t*)block - 1;
    /* Check if already used block is of given size or less than it then return, this is possible because one may want to reduce size. */
    if (header->size >= size)
        return block;
    ret = custom_malloc(size);
    
    if (ret) {
        /* copy content from previous one */
        memcpy(ret, block, header->size);
        /* Free up old block */
        custom_free(block);
    }
    
    return ret;
}