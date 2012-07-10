
#ifndef TEST_H_
#define TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../riack.h"

#define RIAK_TEST_BUCKET "riack_test_bucket"

extern struct RIACK_CLIENT *test_client;
extern struct RIACK_ALLOCATOR test_allocator;

int test_setup(char* host, int port);
void test_teardown();

// Test module functions
int test_get_put(char* testcase);
int test_ping(char* testcase);
int test_bucket(char* testcase);
int test_delete(char* testcase);
int test_mapred(char* testcase);
int test_clientid(char* testcase);
int test_misc(char* testcase);

// Specific tests
int test_bucket_list();
int test_get1();
int test_put1();
int test_put_return_header();
int test_delete_basic();
int test_ping_basic();
int test_bucket_list();
int test_delete_basic();
int test_mapred_basic();
int test_clientid_basic();
int test_server_info();

// Helpers
int put(char* key, char* data);
int delete(char* key);

#endif /* TEST_H_ */
