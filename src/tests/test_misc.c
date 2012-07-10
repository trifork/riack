
#include "test.h"


int test_misc(char* testcase)
{
	if (strcmp(testcase, "serverinfo") == 0) {
		return test_server_info();
	} else {
		return -1;
	}
}


int test_server_info() {
	RIACK_STRING node, version;
	if (riack_server_info(test_client, &node, &version) == RIACK_SUCCESS) {
		riack_free_string(test_client, &node);
		riack_free_string(test_client, &version);
		return 0;
	}
	return 1;
}
