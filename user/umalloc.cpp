#include "user/user.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

#ifdef __cplusplus
extern "C" {
#endif


#define HEADER_SIZE 8

#define MIN_ALLOC_SIZE_SHIFT 4
#define MAX_ALLOC_SIZE_SHIFT 26

// Min alloc size is 16 Byte and max alloc size is 64 MiB because max Memory is 128
#define MIN_ALLOC_SIZE ((size_t) 1 << MIN_ALLOC_SIZE_SHIFT)
#define MAX_ALLOC_SIZE ((size_t) 1 << MAX_ALLOC_SIZE_SHIFT)


#define BUCKET_AMOUNT (MAX_ALLOC_SIZE_SHIFT - MIN_ALLOC_SIZE_SHIFT + 1)

#define NULL nullptr

/*
Useful Macros for bucket calculations
*/

#define BUCKET_SIZE_FOR_INDEX(IDX) (MAX_ALLOC_SIZE >> IDX)


/*
Doubly linked list to track free blocks for every block size, supports inserting and removing.
*/
typedef struct free_list_s {
    struct free_list_s *next;
    struct free_list_s *prev;
} free_list_t;

void
free_list_init(free_list_t *list)
{
    list->next = list;
    list->prev = list;
}

void
free_list_push(free_list_t *&list, free_list_t *element)
{
    free_list_t *last = list->prev;

    element->next = list;
    element->prev = last;

    list->prev = element;

    last->next = element;

}

free_list_t*
free_list_pop(free_list_t &list)
{
    if(list.next == &list) return NULL;
    free_list_t* last = list.prev;
    free_list_remove(*last);

    return last;
}

void
free_list_remove(free_list_t &element)
{
    element.prev->next = element.next;
    element.next->prev = element.prev;
}


/*
Struct to keep track of binary tree
*/
typedef struct buddy_node_s
{
    uint8 state;
} buddy_node_t;

static buddy_node_t buddy_nodes[1 << (BUCKET_AMOUNT - 1)];

#define LEFT_CHILD_USED 0x1
#define RIGHT_CHILD_USED 0x2

bool_t
buddy_node_exactly_one_child_used(size_t idx)
{
    buddy_node_t node = buddy_nodes[idx];
    bool_t left_used = node.state & LEFT_CHILD_USED;
    bool_t right_used = node.state & RIGHT_CHILD_USED;

    return left_used ^ right_used;
}

bool_t
buddy_node_no_child_used(size_t idx)
{
    buddy_node_t node = buddy_nodes[idx];
    bool_t left_used = node.state & LEFT_CHILD_USED;
    bool_t right_used = node.state & RIGHT_CHILD_USED;

    return !(left_used & right_used);
}

bool_t
buddy_node_both_child_used(size_t idx)
{
    buddy_node_t node = buddy_nodes[idx];
    bool_t left_used = node.state & LEFT_CHILD_USED;
    bool_t right_used = node.state & RIGHT_CHILD_USED;

    return left_used & right_used;
}

/*
Conversion from Node index in buddy tree to pointer to memory.
*/
uint8*
node_to_ptr(size_t node_idx, size_t bucket_idx)
{
    uint64 index_in_bucket = node_idx - (1 << bucket_idx) + 1;
    return base_ptr + index_in_bucket * BUCKET_SIZE_FOR_INDEX(node_idx);
}

size_t
ptr_to_node(uint8* ptr, size_t bucket_idx)
{
    size_t offset = (ptr - base_ptr);
    // index "offset" in current bucket
    size_t local_idx = offset / (BUCKET_SIZE_FOR_INDEX(bucket_idx));
    return ((1 << bucket_idx) - 1) + local_idx;
}


// To not immediately allocate/devide entire Adress-space, start with small limit and then grow it, as is needed
static size_t bucket_index_limit;

/*
Buckets to track free memory blocks of Zweierpotenzen.
Entries in bucket at index zero have size of entire possible Adress space (here 100 MiB)
Buckets from bucket_index_limit to (BUCKET_AMOUNT - 1) are populated, buckets at earlier indices are populated, when more memory is needed
and the tree is grown.
*/ 
static free_list_t buckets[BUCKET_AMOUNT];


static uint8 *base_ptr;

static uint8 *max_ptr;


int
grow_heap_upto(uint8* limit)
{
    if(limit <= max_ptr) return -1;
    if(!sbrk(limit - max_ptr)) return -1;

    max_ptr = limit;
    return 0;
}




#ifdef __cplusplus
}
#endif