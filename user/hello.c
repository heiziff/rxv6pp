#include "kernel/types.h"
#include "user/user.h"


void main(int argc, char** argv)
{

	if (argc < 2) {
		printf("Not enough Args");
		exit(-1);
	}
	printf("Hello World from Group %s\n", argv[1]);
}
