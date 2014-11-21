
#ifndef TEST_H_
#define TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../riack.h"

#define RIAK_TEST_BUCKET "riack_test_bucket"
#define RIAK_TEST_BUCKET_TYPE "riack_bt_test"
#define RIAK_TEST_BUCKET_COMMENTS "riack_test_comments"
#define RIAK_TEST_BUCKET_POSTS    "riack_test_posts"
#define RIAK_TEST_BUCKET_USERS    "riack_test_users"
#define RIAK_TEST_BUCKET_ANSWERS  "riack_test_answers"
#define SZ_IN_RIACK_STR(CSTR, RSTR) RSTR.len = strlen(CSTR); RSTR.value = CSTR;

extern riack_client *test_client;
extern riack_allocator test_allocator;

extern int test_port;
extern char* test_host;

int test_setup(char* host, int port);
void test_teardown();

// Test module functions
int test_get_put(char* testcase);
int test_ping(char* testcase);
int test_bucket(char* testcase);
int test_delete(char* testcase);
int test_mapred(char* testcase);
int test_clientid(char* testcase);
int test_load(char* testcase);
int test_misc(char* testcase);
int test_2i(char* testcase);
int test_meta_links(char* testcase);
int test_search(char* testcase);
int test_ext_props(char* testcase);
int test_crdt(char* testcase);

// Specific tests
int test_bucket_list();
int test_bucket_properties();
int test_bucket_type_props();
int test_get1();
int test_put1();
int test_put_no_key();
int test_put_return_header();
int test_delete_basic();
int test_ping_basic();
int test_bucket_list();
int test_delete_basic();
int test_mapred_basic();
int test_clientid_basic();
int test_server_info();
int test_load_init();
int test_load_cleanup();
int test_last_error();
int test_connect_with_options();
int test_reconnect();
int test_2i_load();
int test_2i_cleanup();
int test_2i_range();
int test_2i_exact();
int test_2i_pagination();
int test_2i_pagination_stream();
int test_meta_links_load();
int test_meta_links_cleanup();
int test_meta_links_metas();
int test_meta_links_links();
int test_large_object();
int test_search1();
int test_ext_bucket_props();
int test_counter();

// Helpers
int test_load_cleanup_bucket(char* szbucket);
int put(char* key, char* data);
int delete(char* key);

int process_file(char* filename, char* target_bucket);
int test_load_cleanup_bucket(char* szbucket);

#endif /* TEST_H_ */
