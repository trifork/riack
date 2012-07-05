
#include "test.h"

#define TEST_KEY_NAME "dummy"
#define TEST_DATA "dummy data"

int test_bucket(char* testcase)
{
	if (strcmp(testcase, "list") == 0) {
		return test_bucket_list();
	} else {
		return -1;
	}
}

int test_bucket_list() {
	size_t num_buckets, i;
	char** bucket_list;
	int result;
	result = 1;
	// First make sure we got a bucket with a name we know
	if (put(TEST_KEY_NAME, TEST_DATA) == RIACK_SUCCESS) {
		if (riack_list_buckets(test_client, &num_buckets, &bucket_list) == RIACK_SUCCESS) {
			// Ok we got a list of buckets back now find the bucket and make sure it exist
			for (i=0; i<num_buckets; ++i) {
				if (strcmp(bucket_list[i], RIAK_TEST_BUCKET) == 0) {
					// If we found the bucket then we must have listed OK
					result = 0;
				}
				free(bucket_list[i]);
			}
			free(bucket_list);
		}
	}
	return result;
}
