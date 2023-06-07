/*!
 *\file
 *\brief test invalid read / writes after mmaping file
*/

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
    int fd = open("tes_t", O_CREATE | O_RDWR);
    assert(fd > 0);

    char content[] = "rasant";
    int size = strlen(content) + 1;

    int w_res = write(fd, content, size);
    assert(w_res == size);

    close(fd);
    fd = open("tes_t", O_RDWR);

    void *va = mmap(0, size, PROT_RW, MAP_SHARED, fd, 0);

    char new_content[] = "galant";
    memcpy(va, new_content, size - 1);

    char buf[size];

    int count = read(fd, buf, size);
    // We shouldn't be able to read, because file is still mmapped
    assert(count <= 0);


    munmap(va, size);

    close(fd);
    fd = open("tes_t", O_RDONLY);

    count = read(fd, buf, size);

    // Check if reading was possible
    assert(count > 0);

    // Check for correct content
    assert(!strcmp(new_content, buf));
}
