
#include "test.h"

#define TEST_NON_EXISTING_KEY "fairly_big_chance_of_not_existing_123321"

int test_misc(char* testcase)
{
	if (strcmp(testcase, "serverinfo") == 0) {
		return test_server_info();
	} else if (strcmp(testcase, "error") == 0) {
		return test_last_error();
	} else {
		return -1;
	}
}

int test_last_error()
{
	RIACK_STRING bucket, key;
	struct RIACK_MAPRED_RESULT *result;
	bucket.len = strlen(RIAK_TEST_BUCKET);
	bucket.value = RIAK_TEST_BUCKET;
	key.len = strlen(TEST_NON_EXISTING_KEY);
	key.value = TEST_NON_EXISTING_KEY;
	// This is just plain wrong!
	if (riack_map_redue(test_client, strlen(RIAK_TEST_BUCKET), RIAK_TEST_BUCKET, APPLICATION_ERLANG_TERM, &result) == RIACK_ERROR_RESPONSE) {
		if (strlen(test_client->last_error) > 0)
			return 0;
	}
	return 1;
}

int test_server_info()
{
	RIACK_STRING node, version;
	if (riack_server_info(test_client, &node, &version) == RIACK_SUCCESS) {
		riack_free_string(test_client, &node);
		riack_free_string(test_client, &version);
		return 0;
	}
	return 1;
}
