/*! \file user.h
 * \brief header for userspace standard library
 */

#ifndef INCLUDED_user_bmalloc_h
#define INCLUDED_user_bmalloc_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*!
 * \brief block allocator struct
 * wraps pointer, size and alignment
 */
struct block {
    void *begin;
    uint32_t size;
    uint32_t align;
};
typedef struct block block;

#define BALLOC(T,N) block_alloc(sizeof(T)*(N), alignof(T))

block block_alloc(uint32_t size, uint32_t align);

void block_free(block block);
void setup_balloc(void);



void setup_malloc(void);



#ifdef __cplusplus
}

template<typename T>
struct typed_block {
    T *begin;
    uint32_t size;
    uint32_t align;
};

template<typename T>
union typed_block_u {
    block untyped;
    typed_block<T> typed;
};

template<typename T>
inline typed_block_u<T> block_alloc_typed(uint32_t num_elements) {
    return {block_alloc(num_elements * sizeof(T), alignof(T))};
};

#endif

#endif

