#include "user/user.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

#include "user/bmalloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Header size of linked list entry in the free memory blocks
#define HEADER_SIZE 8

#define MIN_ALLOC_SIZE_SHIFT 4
#define MAX_ALLOC_SIZE_SHIFT 26

// Min alloc size is 16 Byte and max alloc size is 64 MiB because max Memory is
// 128 MiB
#define MIN_ALLOC_SIZE ((size_t)1 << MIN_ALLOC_SIZE_SHIFT)
#define MAX_ALLOC_SIZE ((size_t)1 << MAX_ALLOC_SIZE_SHIFT)

#define BUCKET_AMOUNT (MAX_ALLOC_SIZE_SHIFT - MIN_ALLOC_SIZE_SHIFT + 1)

    /*
    Useful Macros for bucket calculations
    */

#define BUCKET_SIZE_FOR_INDEX(IDX) (MAX_ALLOC_SIZE >> (IDX))

    /*
    Doubly linked list to track free blocks for every block size, supports
    inserting (inserts at end), popping (pops from end) and removing (removes
    from end).
    */
    typedef struct free_list_s
    {
        struct free_list_s *next;
        struct free_list_s *prev;
    } free_list_t;

    void
    free_list_init(free_list_t &list)
    {
        list.next = &list;
        list.prev = &list;
    }

    void
    free_list_push(free_list_t &list, free_list_t *element)
    {
        free_list_t *last = list.prev;

        element->next = &list;
        element->prev = last;

        list.prev = element;

        last->next = element;
    }

    void
    free_list_remove(free_list_t &element)
    {
        element.prev->next = element.next;
        element.next->prev = element.prev;
    }

    free_list_t *
    free_list_pop(free_list_t &list)
    {
        if (list.prev == &list)
            return NULL;
        free_list_t *last = list.prev;
        free_list_remove(*last);

        return last;
    }

#define LEFT_CHILD_USED 0x1
#define RIGHT_CHILD_USED 0x2

#define GET_PARENT(IDX) (((IDX)-1) / 2)
#define GET_LEFT_CHILD(IDX) (2 * (IDX) + 1)
#define GET_RIGHT_CHILD(IDX) (2 * (IDX) + 2)

#define GET_BUDDY(IDX) ((((IDX)-1) ^ 1) + 1)

    // To not immediately allocate/devide entire Adress-space, start with small
    // limit and then grow it, as is needed
    static size_t root_bucket_index;

    /*
    Buckets to track free memory blocks of Zweierpotenzen.
    Entries in bucket at index zero have size of entire possible Adress space
    (here MAX_ALLOC_SIZE MiB) Buckets from bucket_index_limit to (BUCKET_AMOUNT - 1) are
    populated, buckets at earlier indices are populated, when more memory is
    needed and the tree is grown.
    */
    static free_list_t buckets[BUCKET_AMOUNT];

    static uint8 *base_ptr;

    static uint8 *max_ptr;

    /*
    Struct to keep track of binary tree nodes. Node contains information about
    children
    */
    typedef struct buddy_node_s
    {
        uint8 state;
    } buddy_node_t;

    static buddy_node_t buddy_nodes[1 << (BUCKET_AMOUNT - 1)];

    /*
    Returns true, if the bit in the state of the node at index "idx"
    */
    bool_t
    buddy_node_get_bit(size_t idx, uint8 bitmask)
    {
        return ((buddy_nodes[idx].state & bitmask) != 0);
    }

    /*
    Flips the given bit in the state of the node at index "idx"
    */
    void
    buddy_node_flip_bit(size_t idx, uint8 bitmask)
    {
        buddy_nodes[idx].state ^= bitmask;
    }

    /*
    Returns true, if exactly one child of a given node at index "idx" is used
    */
    bool_t
    buddy_node_exactly_one_child_used(size_t idx)
    {
        buddy_node_t node = buddy_nodes[idx];
        bool_t left_used = (node.state & LEFT_CHILD_USED) != 0;
        bool_t right_used = (node.state & RIGHT_CHILD_USED) != 0;

        return left_used ^ right_used;
    }

    /*
    Returns true, if both children of a given node at index "idx" aren't used
    */
    bool_t
    buddy_node_no_child_used(size_t idx)
    {
        buddy_node_t node = buddy_nodes[idx];
        bool_t left_used = (node.state & LEFT_CHILD_USED) != 0;
        bool_t right_used = (node.state & RIGHT_CHILD_USED) != 0;

        return !(left_used & right_used);
    }

    /*
    Returns true, if both children of a given node at index "idx" are used (e.g.
    have memory allocated)
    */
    bool_t
    buddy_node_both_child_used(size_t idx)
    {
        buddy_node_t node = buddy_nodes[idx];
        bool_t left_used = (node.state & LEFT_CHILD_USED) != 0;
        bool_t right_used = (node.state & RIGHT_CHILD_USED) != 0;

        return left_used & right_used;
    }

    /*
    Conversion from node index in binary tree to pointer to memory.
    */
    uint8 *
    node_to_ptr(size_t node_idx, size_t bucket_idx)
    {
        // TODO: understand and refactor
        return base_ptr + ((node_idx - (1 << bucket_idx) + 1) << (MAX_ALLOC_SIZE_SHIFT - bucket_idx));
    }

    /*
   Conversion from pointer in memory to correct node index in the binary tree.
   Also needs the desired bucket to find the node index
    */
    size_t
    ptr_to_node(uint8 *ptr, size_t bucket_idx)
    {
        size_t offset = (ptr - base_ptr);
        // index "offset" in current bucket
        size_t local_idx = offset / (BUCKET_SIZE_FOR_INDEX(bucket_idx));
        return ((1 << bucket_idx) - 1) + local_idx;
    }

    /*
    Determines the ideal bucket for the requested allocation size.
    */
    size_t
    bucket_for_alloc_size(size_t alloc_size)
    {
        size_t bucket = BUCKET_AMOUNT - 1;
        size_t bucket_size = MIN_ALLOC_SIZE;

        while (bucket_size < alloc_size)
        {
            if (bucket == 0)
                return -1;
            bucket_size *= 2;
            bucket--;
        }

        return bucket;
    }

    /*
    Ask the OS to grow memory up to specific pointer location.
    */
    int
    grow_heap_upto(uint8 *limit)
    {
        if (limit <= max_ptr)
            return 0;

        char *old = sbrk(limit - max_ptr);
        if (!old)
            return -1;

        max_ptr = limit;
        return 0;
    }

    /*
    Attempts to grow binary tree. Two options:
    1.  Both children are empty -> grow and merge and insert parent into freelist
    2.  Left child has memory allocated -> grow and only insert right child into
    freelist

    CALLER HAS TO VERIFY VALIDITY OF PARAMETER "rank"
    */
    int
    grow_tree_upto(size_t rank)
    {
        if (rank < 0)
            return -1;

        // Root is always at base pointer
        size_t root = ptr_to_node(base_ptr, root_bucket_index);

        // we can't grow the tree if the root is already at node 0
        if (root == 0)
            return 0;

        size_t parent = GET_PARENT(root);

        // OPTION 1: Both children are empty:
        if (!buddy_node_get_bit(parent, LEFT_CHILD_USED))
        {
            //  Increase size every time and add to parents freelist until we are at
            //  desired rank
            while (root_bucket_index > rank)
            {
                // Parent has its right child used bit initialized to 0. No need to
                // set roots brother to unused

                // Since root unused, it is only entry in highest rank of freelist.
                // We can safely pop here
                free_list_pop(buckets[root_bucket_index]);

                // New root index is now one smaller.
                root_bucket_index--;

                // Insert new freelist entry to parent
                free_list_init(buckets[root_bucket_index]);
                free_list_push(buckets[root_bucket_index],
                               (free_list_t *)base_ptr);

                // Get new values for root and parent
                root = ptr_to_node(base_ptr, root_bucket_index);
                parent = GET_PARENT(root);
            }

            return 0;
        }

        // OPTION 2: Root is used! We can't merge and only insert right child
        else
        {
            while (root_bucket_index > rank)
            {
                uint8 *right_buddy = node_to_ptr(GET_BUDDY(root), root_bucket_index);
                if (grow_heap_upto(right_buddy + sizeof(free_list_t)) < 0)
                    return -1;

                free_list_push(buckets[root_bucket_index], (free_list_t *)right_buddy);

                root_bucket_index--;

                free_list_init(buckets[root_bucket_index]);

                root = ptr_to_node(base_ptr, root_bucket_index);
                parent = GET_PARENT(root);

                // Set in parent, that (old) root is currently used
                if (root != 0)
                {
                    buddy_node_flip_bit(parent, LEFT_CHILD_USED);
                }
            }
        }
        // Check if parent has children, that are used (aka us :D)

        return 0;
    }

    void *
    malloc(uint size)
    {
        if (size == 0)
            return NULL;

        if (size + HEADER_SIZE > MAX_ALLOC_SIZE)
            return NULL;

        // If base pointer is NULL, this is the first call to malloc and the
        // initial memory chunk must be claimed and added to freelist
        if (!base_ptr)
        {
            // Use cheese to find initial base pointer and set max_ptr to same
            // value
            base_ptr = (uint8 *)sbrk(0);
            if ((uint64)(base_ptr + HEADER_SIZE) & (PAGE_SIZE - 1))
            {
                printf("growing stack for malloc by: %d\n", PAGE_SIZE - ((uint64)base_ptr % PAGE_SIZE) - HEADER_SIZE);
                char* old = sbrk(PAGE_SIZE - ((uint64)base_ptr % PAGE_SIZE) - HEADER_SIZE);
                base_ptr = (uint8 *)sbrk(0);
                printf("from old %p to %p\n", old, base_ptr);
            }

            max_ptr = base_ptr;

            root_bucket_index = BUCKET_AMOUNT - 1;

            grow_heap_upto(base_ptr + sizeof(free_list_t));

            free_list_init(buckets[root_bucket_index]);
            free_list_push(buckets[root_bucket_index], (free_list_t *)base_ptr);
        }

        // Find ideal bucket to fit our allocation, DON'T FORGET THAT HEADER!
        ssize_t ideal_bucket = bucket_for_alloc_size(size + HEADER_SIZE);

        // Make sure that we have at least tree for ideal bucket
        if (grow_tree_upto(ideal_bucket) != 0)
            return NULL;

        // Early out, if bucket with ideal size has elements
        // Write allocated size into header
        uint8 *please = (uint8 *)free_list_pop(buckets[ideal_bucket]);

        if (please != NULL)
        {
            grow_heap_upto(please + BUCKET_SIZE_FOR_INDEX(ideal_bucket));
            size_t node = ptr_to_node(please, ideal_bucket);
            uint8 bitmask = node % 2 == 0 ? RIGHT_CHILD_USED : LEFT_CHILD_USED;
            buddy_node_flip_bit(GET_PARENT(node), bitmask);
            *((size_t *)please) = size;
            return please + HEADER_SIZE;
        }

        // Now we aren't happy anymore
        // Find free bucket
        // current bucket is the bucket in which the please block is found (hopefully)
        ssize_t current_bucket = ideal_bucket;
        while (current_bucket >= 0)
        {
            if (grow_tree_upto(current_bucket) != 0)
            {
                return NULL;
            }

            please = (uint8 *)free_list_pop(buckets[current_bucket]);

            // Get out if we found a bucket!
            if (please != NULL)
            {
                break;
            }

            if (current_bucket == 0 || current_bucket > (ssize_t)root_bucket_index)
            {
                current_bucket--;
                continue;
            }

            if (grow_tree_upto(current_bucket - 1) != 0)
                return NULL;

            please = (uint8 *)free_list_pop(buckets[current_bucket]);
            break;
        }

        // Not enough memory for allocation :(
        if (current_bucket < 0 || please == NULL)
        {
            printf("[ERROR] malloc: cannot allocate more memory\n");
            return NULL;
        }

        // Make sure, that there is enough space for at least the list entry of
        // current_bucket right child, as he will be added to the free list, while
        // we continue down the left children
        grow_heap_upto(please + BUCKET_SIZE_FOR_INDEX(current_bucket + 1) + sizeof(free_list_t));

        // Now split larger bucket until we small space
        while (current_bucket < ideal_bucket)
        {
            size_t big_block_idx = ptr_to_node(please, current_bucket);

            buddy_node_flip_bit(big_block_idx, LEFT_CHILD_USED);

            // Halve bucket size
            current_bucket++;

            uint8 *right_child = node_to_ptr(GET_RIGHT_CHILD(big_block_idx), current_bucket);

            free_list_push(buckets[current_bucket], (free_list_t *)right_child);
        }

        // Found block, write size and return correct adress, offset by header
        *((size_t *)please) = (size_t)size;

        return please + HEADER_SIZE;
    }

    void
    free(void *ptr_to_free)
    {
        if (ptr_to_free == NULL)
            return;

        uint8 *ptr = (uint8 *)ptr_to_free;
        // Get header content
        uint64 alloc_size = *((uint64 *)(ptr - HEADER_SIZE));

        // Find alloced bucket size
        size_t bucket = bucket_for_alloc_size(alloc_size + HEADER_SIZE);
        size_t node_idx = ptr_to_node(ptr, bucket);

        while (bucket > root_bucket_index)
        {
            size_t parent = GET_PARENT(node_idx);

            // Check if we are left or right child based on our index
            uint8 bitmask = node_idx % 2 == 0 ? RIGHT_CHILD_USED : LEFT_CHILD_USED;
            buddy_node_flip_bit(parent, bitmask);

            // we are unused, so if buddy is used then we cannot merge and are done
            if (buddy_node_exactly_one_child_used(node_idx))
            {
                free_list_push(buckets[bucket],
                               (free_list_t *)node_to_ptr(node_idx, bucket));
                return;
            }

            // Remove our now CONFIRMED free buddy from the freelist, as we are
            // merging our blocks.
            // Merged block will then be inserted into larger bucket later
            free_list_remove(
                *(free_list_t *)node_to_ptr(GET_BUDDY(node_idx), bucket));

            bucket--;
            node_idx = parent;
        }
        free_list_push(buckets[bucket],
                       (free_list_t *)node_to_ptr(node_idx, bucket));
    }

    // BEGIN BLOCK ALLOC IMPLEMENTATION

    // We build a block allocation implementation on top of the buddy allocator used by malloc.
    // This way we can reuse and even combine both allocation strategies and still have decent
    // allocation speeds.
    block
    block_alloc(uint32 size, uint32 align)
    {
        block b;

        if (size == 0)
            return b;

        // To ensure we have enough space for the aligned block allocation, we request the
        // maximum amount of memory needed to fit the allocation + alignment
        uint maximum_size_needed = size + align;
        void *ptr = malloc(maximum_size_needed);

        b.begin = ptr;
        b.size = size;
        b.align = align;

        size_t adr = (size_t)ptr;
        if (adr % align == 0)
            return b;

        // in case the bucket ptr is not already aligned to "align" we need to move the begin
        // of the block so it is aligned again.
        b.begin = (void *)(adr + align - (adr % align));
        return b;
    }

    void
    block_free(block b)
    {
        if (b.begin == 0)
            return;

        // figure out which bucket size was used for the allocation
        uint allocated_size = b.size + b.align;
        size_t bucket_idx = bucket_for_alloc_size(allocated_size + HEADER_SIZE);

        // little hacky: we want to get from the begin ptr of the block to the begin of its
        // bucket. We can find the node index of the used bucket with the block begin ptr
        // and can then convert the node index back to the start ptr of the bucket
        void *bucket_ptr = node_to_ptr(ptr_to_node((uint8 *)b.begin, bucket_idx), bucket_idx);

        // free takes the ptr to right after the header
        free((uint8 *)bucket_ptr + HEADER_SIZE);
    }

    // END BLOCK ALLOC IMPL

#ifdef __cplusplus
}
#endif
