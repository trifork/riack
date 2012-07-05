
#include "test.h"

int test_ping(char* testcase)
{
	if (strcmp(testcase, "basic") == 0) {
		return test_ping_basic();
	} else {
		return -1;
	}
}

int test_ping_basic() {
	if (riack_ping(test_client) == RIACK_SUCCESS) {
		printf("<- Pong\n");
	} else {
		return -3;
	}
	return 0;
}
