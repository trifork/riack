
#include "test.h"

#define TEST_NON_EXISTING_KEY "fairly_big_chance_of_not_existing_123321"

int test_misc(char* testcase)
{
	if (strcmp(testcase, "serverinfo") == 0) {
		return test_server_info();
	} else if (strcmp(testcase, "error") == 0) {
		return test_last_error();
	} else if (strcmp(testcase, "options") == 0) {
		return test_connect_with_options();
	} else if (strcmp(testcase, "reconnect") == 0) {
		return test_reconnect();
	} else if (strcmp(testcase, "large") == 0) {
			return test_large_object();
	} else {
		return -1;
	}
}

int test_reconnect()
{
	riack_client *client;
	riack_connection_options options;
	int result;
	result = 1;
	client = riack_new_client(0);
	options.recv_timeout_ms = 1500;
	options.send_timeout_ms = 1500;
    options.keep_alive_enabled = 1;
	if (riack_connect(client, test_host, test_port, &options) == RIACK_SUCCESS) {
		riack_timeout_test(client);
		if (riack_reconnect(client) == RIACK_SUCCESS) {
			result = 0;
		}
	}
	riack_free(client);
	return result;
}

int test_connect_with_options()
{
	riack_client *client;
	riack_connection_options options;
	int result;
	result = 1;
	client = riack_new_client(0);
	options.recv_timeout_ms = 1500;
	options.send_timeout_ms = 1500;
	if (riack_connect(client, test_host, test_port, &options) == RIACK_SUCCESS) {
		result = 0;
	}
	riack_free(client);
	return result;
}

int test_large_object()
{
	riack_get_object *obj;
	riack_string key, bucket;
	char* largeObject;
	int result;

	result = 1;

	key.value = "LARGE_OBJECT";
	key.len = strlen(key.value);
	bucket.value = RIAK_TEST_BUCKET;
	bucket.len = strlen(bucket.value);

	largeObject = malloc(64*1024);
	memset(largeObject, '#', 64*1024);
	if (put(key.value, largeObject) == RIACK_SUCCESS) {
		if (riack_get(test_client, &bucket, &key, 0, &obj) == RIACK_SUCCESS) {
			// Validate the content we got back
			if ((obj->object.content_count == 1) &&
				(obj->object.content[0].data_len == 64*1024)) {
				result = 0;
			}
		}
        riack_free_get_object_p(test_client, &obj);
	} else {

	}
	free(largeObject);
	delete(key.value);
	return result;
}

int test_last_error()
{
    riack_string bucket, key;
    riack_mapred_response_list *result;
	bucket.len = strlen(RIAK_TEST_BUCKET);
	bucket.value = RIAK_TEST_BUCKET;
	key.len = strlen(TEST_NON_EXISTING_KEY);
	key.value = TEST_NON_EXISTING_KEY;
	// This is just plain wrong! (which is the general idea when testing error handling)
    if (riack_map_reduce(test_client, strlen(RIAK_TEST_BUCKET), (uint8_t*)RIAK_TEST_BUCKET, APPLICATION_ERLANG_TERM, &result) == RIACK_ERROR_RESPONSE) {
		if (strlen(test_client->last_error) > 0)
			return 0;
	}
	return 1;
}

int test_server_info()
{
	riack_string *node, *version;
	if (riack_server_info(test_client, &node, &version) == RIACK_SUCCESS) {
		riack_free_string_p(test_client, &node);
		riack_free_string_p(test_client, &version);
		return 0;
	}
	return 1;
}
