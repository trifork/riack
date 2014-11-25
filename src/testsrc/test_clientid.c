
#include "test.h"

#define TEST_CLIENT_ID "T3ST"

int test_clientid(char* testcase)
{
	if (strcmp(testcase, "basic") == 0) {
		return test_clientid_basic();
	} else {
		return -1;
	}
}

int test_clientid_basic() {
	riack_string clientid_in, *clientid_out;
	clientid_in.len = strlen(TEST_CLIENT_ID);
	clientid_in.value = TEST_CLIENT_ID;
	if (riack_set_clientid(test_client, &clientid_in) == RIACK_SUCCESS) {
		if (riack_get_clientid(test_client, &clientid_out) == RIACK_SUCCESS) {
			if (clientid_in.len != clientid_out->len)
				return 2;
			if (memcmp(clientid_in.value, clientid_out->value, clientid_in.len) == 0) {
				riack_free_string_p(test_client, &clientid_out);
				return 0;
			}
		}
	}
	return 1;
}
