
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
	riack_string_list *buckets;
	int result, list_buckets_result;
	result = 1;

	// First make sure we got a bucket with a name we know
	if (put(TEST_KEY_NAME, TEST_DATA) == RIACK_SUCCESS) {
        list_buckets_result = riack_list_buckets(test_client, &buckets);
        if (list_buckets_result == RIACK_SUCCESS) {
			// Ok we got a list of buckets back now find the bucket and make sure it exist
			for (i=0; i<buckets->string_count; ++i) {
				if (memcmp(buckets->strings[i].value, RIAK_TEST_BUCKET, buckets->strings[i].len) == 0) {
					// If we found the bucket then we must have listed OK
					result = 0;
				}
			}
            riack_free_string_list_p(test_client, &buckets);
		}
	}
	return result;
}
