
#include "test.h"

#define TEST_KEY1 "testkey1"
#define TEST_DATA1 "testdata1"

int test_get_put(char* testcase)
{
	if (strcmp(testcase, "put1") == 0) {
		return test_put1();
	} else if (strcmp(testcase, "put2") == 0) {
		return test_put_return_header();
    } else if (strcmp(testcase, "put3") == 0) {
        return test_put_no_key();
    } else if (strcmp(testcase, "get1") == 0) {
		return test_get1();
	} else {
		return -1;
	}
}

int put(char* key, char* data)
{
    return riack_put_simple(test_client, RIAK_TEST_BUCKET, key, (uint8_t*)data, strlen(data), "text/plain");
}

int test_put_no_key()
{
    riack_object obj, *put_result;
    char* data;
    int result;

    result = 1;

    data = "{\"testvalue\": \"plappe lappe 2\"}";
    obj.bucket.value = TEST_DATA1;
    obj.bucket.len = strlen(TEST_DATA1);
    obj.key.value = 0;
    obj.key.len = 0;
    obj.vclock.len = 0;
    obj.content = (riack_content*)RMALLOC(test_client, sizeof(riack_content));
    memset(obj.content, 0, sizeof(riack_content));
    obj.content[0].content_type.value = "application/json";
    obj.content[0].content_type.len = strlen(obj.content[0].content_type.value);
    obj.content[0].data = (uint8_t*)data;
    obj.content[0].data_len = strlen(data);

    if (riack_put(test_client, &obj, &put_result, (riack_put_properties*)0) == RIACK_SUCCESS) {
        if (put_result->key.len > 0) {
            result = 0;
        }
        riack_free_object_p(test_client, &put_result);
        RFREE(test_client, obj.content);
    }
    return result;
}

int test_put1()
{
	int result;
	result = 1;
	if (put(TEST_KEY1, TEST_DATA1) == RIACK_SUCCESS) {
		result = 0;
	}
	delete(TEST_KEY1);
	return result;
}

int test_put_return_header()
{
	char* data;
	size_t cnt, i;
	int result;
	riack_object obj, *put_result;
    riack_put_properties put_props;
	result = 1;

	memset(&put_props, 0, sizeof(put_props));
	put_props.return_body_use = 1;
	put_props.return_body = 1;

	data = "{\"testvalue\": \"plappe lappe 2\"}";

	obj.bucket.value = RIAK_TEST_BUCKET;
	obj.bucket.len = strlen(RIAK_TEST_BUCKET);
	obj.key.value = "test_put2";
	obj.key.len = strlen(obj.key.value);
	obj.vclock.len = 0;
	obj.content_count = 1;
	obj.content = (riack_content*)malloc(sizeof(riack_content));
	memset(obj.content, 0, sizeof(riack_content));
	obj.content[0].content_type.value = "application/json";
	obj.content[0].content_type.len = strlen(obj.content[0].content_type.value);
    obj.content[0].data = (uint8_t*)data;
	obj.content[0].data_len = strlen(data);

	if (riack_put(test_client, &obj, &put_result, &put_props) == RIACK_SUCCESS) {
		cnt = put_result->content_count;
		if (cnt == 1) {
			if (put_result->content[0].content_type.len == obj.content[0].content_type.len) {
				// Make sure content type is the same
				if (memcmp( put_result->content[0].content_type.value,
							obj.content[0].content_type.value,
							obj.content[0].content_type.len) == 0) {
					// Make sure the actual content is the same
					i = put_result->content[0].data_len;
					if (memcmp(data, put_result->content[0].data, i) == 0) {
						result = 0;
					}
				}
			}
		}
        riack_free_object_p(test_client, &put_result);
	}
	free(obj.content);
	delete("test_put2");
	return result;
}

int test_get1()
{
	riack_get_object *obj;
	riack_string key, bucket;
	char *data;
	size_t content_size;
	int result;
	result = 1;
	key.value = "test_get_key1";
	key.len = strlen(key.value);
	bucket.value = RIAK_TEST_BUCKET;
	bucket.len = strlen(bucket.value);
	data = "test content 1234";

	if (put(key.value, data) == RIACK_SUCCESS) {
		if (riack_get(test_client, &bucket, &key, 0, &obj) == RIACK_SUCCESS) {
			// Validate the content we got back
			if ((obj->object.content_count == 1) &&
				(obj->object.content[0].data_len == strlen(data))) {
				content_size = obj->object.content[0].data_len;
				if (memcmp(obj->object.content[0].data, data, strlen(data)) == 0) {
					result = 0;
				}
			}
		}
        riack_free_get_object_p(test_client, &obj);
	}
	delete(key.value);
	return result;
}

