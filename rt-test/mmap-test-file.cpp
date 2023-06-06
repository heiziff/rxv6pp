/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main()
{
    int fd = open("tes_t.txt", O_CREATE | O_RDWR);
    if (fd < 0)
    {
        printf("rip\n");
        return;
    }

    char string[] = "Hallo mein Name ist Keko";
    if (write(fd, string, strlen(string) + 1) != (int)strlen(string) + 1)
    {
        printf("write failed\n");
        return;
    }

    close(fd);
    fd = open("tes_t.txt", O_RDONLY);

    void *va = mmap((void *)0, strlen(string) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    assert(strcmp((char *)va, string) == 0);

    munmap(va, strlen(string) + 1);
    close(fd);

    char *string2 = (char *)malloc(2 * 4096 + 2);
    memset(string2, 'A', 2 * 4096 + 1);
    string2[2 * 4096 + 1] = 0;

    fd = open("tes_t2.txt", O_CREATE | O_RDWR);
    if (fd < 0)
    {
        printf("rip\n");
        return;
    }

    if (write(fd, string2, strlen(string2) + 1) != (int)strlen(string2) + 1)
    {
        printf("write failed\n");
        return;
    }

    close(fd);
    fd = open("tes_t2.txt", O_RDONLY);

    va = mmap((void *)0, strlen(string2) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    assert(strcmp((char *)va, string2) == 0);

    munmap(va, strlen(string2) + 1);
    close(fd);
    free(string2);

    char test_buf[] = "AAAABBBB";
    char result[] = "AAAA";

    fd = open("tes_t.txt", O_CREATE | O_RDWR);
    if (fd < 0)
        printf("rip\n");

    write(fd, test_buf, strlen(test_buf) + 1);
    close(fd);
    fd = open("tes_t.txt", O_RDONLY);

    va = mmap((void *)0, 5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ((char *)va)[4] = 0;
    assert(strcmp((char *)va, result) == 0);

    munmap(va, 4);
    close(fd);

    assert(!unlink("tes_t.txt"));
    assert(!unlink("tes_t2.txt"));

}
