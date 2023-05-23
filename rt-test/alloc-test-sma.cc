/*!
 * \file
 * \brief test small allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void test_write(void *data, int bytes)
{
    printf("bobo\n");
    auto ptr = reinterpret_cast<char *>(data);
    printf("baba\n");
    for (int i = 0; i < bytes; ++i)
    {
        printf("writing offset %d\n", i);
        ptr[i] = i + bytes;
    }
    for (int i = 0; i < bytes; ++i)
        assert(ptr[i] == bytes + i);
}

void main()
{
    for (int i = 1; i <= 5; ++i)
    {
        auto bytes = 1 << i;

        auto malloced = malloc(bytes);
        printf("malloced %p\n", malloced);
        assert(malloced);
        // *(uint8 *)malloced = 4;
        printf("33\n");
        test_write(malloced, i);
        printf("1\n");

        auto malloced2 = malloc(bytes);
        printf("malloced %p\n", malloced);
        assert(malloced);
        // *(uint8 *)malloced = 4;
        printf("33\n");
        test_write(malloced, i);
        printf("1\n");

        // auto block = block_alloc(bytes, bytes < 16 ? bytes : 16);
        // assert(block.begin);
        // test_write(block.begin, i);
        // printf("2\n");

        // auto block2 = block_alloc(bytes, bytes < 16 ? bytes : 16);
        // assert(block.begin);
        // test_write(block.begin, i);

        // free(malloced);
        printf("3\n");
        free(malloced);
        free(malloced2);
        printf("4\n");
    }
}
