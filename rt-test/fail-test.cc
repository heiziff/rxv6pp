#include "user/user.h"
#include "rt-test/assert.h"

int main() {
	printf("gonna crash!\n");
	assert(false);
}
