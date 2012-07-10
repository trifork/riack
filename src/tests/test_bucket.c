
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
	size_t i;
	RIACK_STRING_LIST buckets;
	int result;
	result = 1;
	// First make sure we got a bucket with a name we know
	if (put(TEST_KEY_NAME, TEST_DATA) == RIACK_SUCCESS) {
		if (riack_list_buckets(test_client, &buckets) == RIACK_SUCCESS) {
			// Ok we got a list of buckets back now find the bucket and make sure it exist
			for (i=0; i<buckets.string_count; ++i) {
				if (memcmp(buckets.strings[i].value, RIAK_TEST_BUCKET, buckets.strings[i].len) == 0) {
					// If we found the bucket then we must have listed OK
					result = 0;
				}
			}
			riack_free_string_list(test_client, &buckets);
		}
	}
	return result;
}
