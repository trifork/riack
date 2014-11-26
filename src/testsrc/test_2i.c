
#include "test.h"

#define TEST_2i_BUCKET "test_bucket_2i"
#define TETS_2i_INDEX1 "index1_int"
#define TETS_2i_INDEX2 "index2_bin"
#define TEST_DUMMY_VALUE "1 2 3 test 6 7 8"

// Make sure this value
#define TEST_2i_ENTRIES_X10 10

int test_2i(char* testcase)
{
	if (strcmp(testcase, "load") == 0) {
		return test_2i_load();
	} else if (strcmp(testcase, "exact") == 0) {
		return test_2i_exact();
	} else if (strcmp(testcase, "range") == 0) {
		return test_2i_range();
	} else if (strcmp(testcase, "cleanup") == 0) {
		return test_2i_cleanup();
    } else if (strcmp(testcase, "pagination") == 0) {
        return test_2i_pagination();
    } else if (strcmp(testcase, "stream_pag") == 0) {
        return test_2i_pagination_stream();
    } else {
		return -1;
	}
}

int put_object_with_index(char* bucket, char* key, char*value, riack_pair *indexes, size_t index_count)
{
	riack_content content;
	riack_object object;
	
	memset(&content, 0, sizeof(content));
	memset(&object, 0, sizeof(object));
	
	object.bucket.value = bucket;
	object.bucket.len = strlen(bucket);
	object.key.value = key;
	object.key.len = strlen(key);
	object.content_count = 1;
	object.content = &content;
	content.content_type.value = "plain/text";
	content.content_type.len = strlen(content.content_type.value);
	content.index_count = index_count;
	content.indexes = indexes;
	content.data = (uint8_t*)value;
	content.data_len = strlen(value);
	if (riack_put(test_client, &object, 0, 0) == RIACK_SUCCESS) {
		return 1;
	}
	return 0;
}

int test_2i_load()
{
	char buffer1[10], buffer2[10], keybuffer[16];
	riack_pair *indexes;
	int i;
	indexes = malloc(sizeof(riack_pair)*2);
	indexes[0].key.value = TETS_2i_INDEX1;
	indexes[0].key.len = strlen(indexes[0].key.value);
	indexes[0].value_present = 1;
	indexes[1].key.value = TETS_2i_INDEX2;
	indexes[1].key.len = strlen(indexes[1].key.value);
	indexes[1].value_present = 1;
	
	for (i=0; i<TEST_2i_ENTRIES_X10*10; ++i) {
		sprintf(buffer1, "%d", i);
		sprintf(buffer2, "%d", i % 10);
		sprintf(keybuffer, "key%d", i);
		indexes[0].value_len = strlen(buffer1);
		indexes[0].value = (uint8_t*)buffer1;
		indexes[1].value_len = strlen(buffer2);
		indexes[1].value = (uint8_t*)buffer2;
        if (!put_object_with_index(TEST_2i_BUCKET, keybuffer, TEST_DUMMY_VALUE, indexes, 2)) {
			return 1;
		}
	}
	free(indexes);
	return 0;
}

void test_2i_pagination_stream_cb(riack_client* c, void* arg, riack_string *key) {
    int *cnt = (int*)arg;
    (*cnt)++;
}

int test_2i_pagination_stream() {
    char min_buff[10], max_buff[10];
    riack_2i_query_req req;
    riack_string *continuation;
    int result, cnt;
    memset(&req, 0, sizeof(req));
    result = 1;
    cnt = 0;
    req.bucket.len = strlen(TEST_2i_BUCKET);
    req.bucket.value = TEST_2i_BUCKET;
    req.index.len = strlen(TETS_2i_INDEX1);
    req.index.value = TETS_2i_INDEX1;
    req.max_results = 25;
    // from 5 to 50 inc. => 46 keys
    sprintf(min_buff, "%d", 5);
    sprintf(max_buff, "%d", 50);
    req.search_min.len = strlen(min_buff);
    req.search_min.value = min_buff;
    req.search_max.len = strlen(max_buff);
    req.search_max.value = max_buff;
    if (riack_2i_query_stream(test_client, &req, &continuation, &test_2i_pagination_stream_cb, &cnt) == RIACK_SUCCESS) {
        if (cnt == 25 && continuation) {
            riack_string *continuation_inner;
            result = 0;
            // We got the first five starting from 5-9 now get the rest which should be 8
            req.continuation_token = *continuation;
            req.max_results = 100;

            if (riack_2i_query_stream(test_client, &req, &continuation_inner, &test_2i_pagination_stream_cb, &cnt) == RIACK_SUCCESS) {
                // Expect 4 keys since we got 5 for and need 9 in total
                if (cnt == 46 && continuation_inner == 0) {
                    result = 0;
                }
            }
        }
        if (continuation) {
            riack_free_string_p(test_client, &continuation);
        }
    }
    return result;
}

int test_2i_pagination() {
    char min_buff[10], max_buff[10];
    riack_2i_query_req req;
    int result;
    riack_string_list *keys;
    riack_string *continuation_out;
    memset(&req, 0, sizeof(req));
    result = 1;
    req.bucket.len = strlen(TEST_2i_BUCKET);
    req.bucket.value = TEST_2i_BUCKET;
    req.index.len = strlen(TETS_2i_INDEX1);
    req.index.value = TETS_2i_INDEX1;
    req.max_results = 5;
    // from 5 to 13 inc. => 9 keys
    sprintf(min_buff, "%d", 5);
    sprintf(max_buff, "%d", 13);
    req.search_min.len = strlen(min_buff);
    req.search_min.value = min_buff;
    req.search_max.len = strlen(max_buff);
    req.search_max.value = max_buff;
    if (riack_2i_query(test_client, &req, &keys, &continuation_out) == RIACK_SUCCESS) {
        if (keys->string_count == 5 && continuation_out) {
            riack_string *continuation_out_inner;
            result = 0;
            req.max_results = 100;
            // Copy continuation token from out to in.
            req.continuation_token = *continuation_out;
            riack_free_string_list_p(test_client, &keys);
            if (riack_2i_query(test_client, &req, &keys, &continuation_out_inner) == RIACK_SUCCESS) {
                // Expect 4 keys since we got 5 for and need 9 in total
                if (keys->string_count == 4 && continuation_out_inner == 0) {
                    result = 0;
                    riack_free_string_list_p(test_client, &keys);
                }
            }
            riack_free_string_p(test_client, &continuation_out_inner);
        }
        riack_free_string_p(test_client, &continuation_out);
    }
    return result;
}

int test_2i_cleanup()
{
	return !test_load_cleanup_bucket(TEST_2i_BUCKET);
}

int test_2i_range()
{
    char min_buff[10], max_buff[10];
    riack_string index1, index2, min_key, max_key, bucket;
	riack_string_list *keys;
    int i, result;
	index1.len = strlen(TETS_2i_INDEX1);
	index1.value = TETS_2i_INDEX1;
	index2.len = strlen(TETS_2i_INDEX2);
	index2.value = TETS_2i_INDEX2;
	bucket.len = strlen(TEST_2i_BUCKET);
	bucket.value = TEST_2i_BUCKET;
	result = 0;
	for (i=0; i<TEST_2i_ENTRIES_X10-5; i += 5) {
		sprintf(min_buff, "%d", i);
		sprintf(max_buff, "%d", (i+4));
		min_key.len = strlen(min_buff);
		min_key.value = min_buff;
		max_key.len = strlen(max_buff);
		max_key.value = max_buff;
		if (riack_2i_query_range(test_client, &bucket, &index1, &min_key, &max_key, &keys) != RIACK_SUCCESS) {
			result = 1;
			break;
		}
		if (keys->string_count != 5) {
			result = 2;
			break;
		}
        riack_free_string_list_p(test_client, &keys);
	}

	return result;
}

int test_2i_exact()
{
	char buffer[10], expected_key[100];
	riack_string index1, index2, search_key, bucket;
	riack_string_list *keys;
	int i, result;
	index1.len = strlen(TETS_2i_INDEX1);
	index1.value = TETS_2i_INDEX1;
	index2.len = strlen(TETS_2i_INDEX2);
	index2.value = TETS_2i_INDEX2;
	bucket.len = strlen(TEST_2i_BUCKET);
	bucket.value = TEST_2i_BUCKET;
	result = 0;
	for (i=0; i<TEST_2i_ENTRIES_X10; ++i) {
		sprintf(buffer, "%d", i);
		sprintf(expected_key, "key%d", i);
		search_key.len = strlen(buffer);
		search_key.value = buffer;

		if (riack_2i_query_exact(test_client, &bucket, &index1, &search_key, &keys) != RIACK_SUCCESS) {
			result = 1;
			break;
		}
		if (keys->string_count != 1 || memcmp(keys->strings[0].value, expected_key, strlen(expected_key)) != 0) {
			result = 2;
			break;
		}
        riack_free_string_list_p(test_client, &keys);
	}
	if (result != 0) {
		return result;
	}
	for (i=0; i<TEST_2i_ENTRIES_X10; ++i) {
		sprintf(buffer, "%d", i);
		search_key.len = strlen(buffer);
		search_key.value = buffer;
		if (riack_2i_query_exact(test_client, &bucket, &index2, &search_key, &keys) != RIACK_SUCCESS) {
			result = 3;
			break;
		}
		if (keys->string_count != TEST_2i_ENTRIES_X10) {
			result = 4;
			break;
		}
        riack_free_string_list_p(test_client, &keys);
	}
	return result;
}
