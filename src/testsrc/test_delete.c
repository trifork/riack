
#include "test.h"

#define TEST_DELETE_KEY "test_delete"

int test_delete(char* testcase)
{
	if (strcmp(testcase, "basic") == 0) {
		return test_delete_basic();
	} else {
		return -1;
	}
}

int delete(char* key) {
	riack_string key_str;
    riack_string bucket;
	key_str.len = strlen(key);
	key_str.value = key;
	bucket.value = RIAK_TEST_BUCKET;
	bucket.len = strlen(bucket.value);
	if (riack_delete(test_client, &bucket, &key_str, 0) == RIACK_SUCCESS) {
		return 0;
	}
	return -1;
}

int test_delete_basic() {
	if (put(TEST_DELETE_KEY, "testvalue") == RIACK_SUCCESS) {
		if (delete(TEST_DELETE_KEY) == 0) {
			return 0;
		}
	}
	return 1;
}
