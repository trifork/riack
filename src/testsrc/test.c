
#include "test.h"

riack_client *test_client;

int test_port;
char* test_host;

int main(int argc, char *argv[])
{
	int result;
	char *module,*test;
	if (argc < 5) {
		fprintf(stderr, "Missing arguments");
		return -1;
	}
	module = argv[1];
	test = argv[2];
	test_host = argv[3];
	test_port = atoi(argv[4]);
	result = -1;
	if (test_setup(test_host, test_port) == 0) {
		if (strcmp(module, "ping") == 0) {
			result = test_ping(test);
		} else if (strcmp(module, "get_put") == 0) {
			result = test_get_put(test);
		} else if (strcmp(module, "bucket") == 0) {
			result = test_bucket(test);
		} else if (strcmp(module, "delete") == 0) {
			result = test_delete(test);
		} else if (strcmp(module, "mapred") == 0) {
			result = test_mapred(test);
		} else if (strcmp(module, "clientid") == 0) {
			result = test_clientid(test);
		} else if (strcmp(module, "2i") == 0) {
			result = test_2i(test);
		} else if (strcmp(module, "meta-links") == 0) {
			result = test_meta_links(test);
        } else if (strcmp(module, "search") == 0) {
            result = test_search(test);
        } else if (strcmp(module, "load") == 0) {
			result = test_load(test);
        } else if (strcmp(module, "ext_props") == 0) {
            result = test_ext_props(test);
        } else if (strcmp(module, "misc") == 0) {
			result = test_misc(test);
        } else if (strcmp(module, "crdt") == 0) {
            result = test_crdt(test);
        }
		test_teardown();
	}
	return result;
}

int test_setup(char* host, int port)
{
	riack_init();
	test_client = riack_new_client(0);
	if (test_client) {
		if (riack_connect(test_client, host, port, 0) != RIACK_SUCCESS) {
			fprintf(stderr, "Failed to connect to RIAK server, make sure the tests is configured corretly in tests/CMakeLists.txt");
			riack_free(test_client);
			return 1;
		}

	}
	return 0;
}

void test_teardown()
{
	riack_free(test_client);
	riack_cleanup();
}



