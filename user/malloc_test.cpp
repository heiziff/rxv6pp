#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char**) {
    void *ps[260];
    size_t i;

    printf("testing allocations... ");
    for (i = 0; i < 260; i++) {
        void* p = malloc(32);
        ps[i] = p;
    }
    printf("OK\n");


    printf("testing freeing... ");
    for (i = 0; i < 260; i++) {
        free(ps[i]);
    }
    printf("OK\n");


    printf("testing block management... ");
    for (i = 0; i < 10; i++) {
        void *p1 = malloc(32);
        free(p1);
        void *p2 = malloc(32);
        free(p2);
        if (p1 != p2) {
            printf("failed\n");
            exit(1);
        }
    }

    for (i = 0; i < 10; i++) {
        void *p1 = malloc(32);
        void *p2 = malloc(32);
        if(p1 == p2) {
            printf("failed\n");
            exit(1);
        }
    }

    void *p1 = malloc(32);
    void *p2 = malloc(32);
    free(p1);
    void *p3 = malloc(32);
    malloc(32);
    malloc(32);
    malloc(32);
    free(p2);
    void *p4 = malloc(32);
    if (p1 != p3 || p2 != p4) {
        printf("failed, invalid block management\n");
        exit(1);
    }
    printf("OK\n");
    

    printf("testing mixed free-list and pool allocations... ");
    char *pool1 = (char*) malloc(32);
    for (i = 0; i < 32; ++i) {
        pool1[i] = 31-i;
    }
    char *free1 = (char*) malloc(500000);
    for (i = 0; i < 500000; ++i) {
        free1[i] = 42;
    }
    char *pool2 = (char*) malloc(32);
    for (i = 0; i < 32; ++i) {
        pool2[i] = 0;
    }
    char *free2 = (char*) malloc(500000);
    for (i = 0; i < 500000; ++i) {
        free2[i] = 0;
    }
    char *pool3 = (char*) malloc(32);
    for (i = 0; i < 32; ++i) {
        pool3[i] = 42;
    }

    for (int i = 0; i < 10; i++) malloc(30);
    free(pool2);
    void *pool4 = malloc(30);
    if (pool2 != pool4) {
        printf("failed, invalid block management\n");
        exit(1);
    }

    free(free1);
    free(free2);
    free(pool1);
    free(pool4);
    free(pool3);
    printf("OK\n");

    return 0;
}

