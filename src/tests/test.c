
#include "test.h"

struct RIACK_CLIENT *test_client;

int main(int argc, char *argv[])
{
	int result;
	char buff[50];
	char *module,*test,*host;
	int port;
	if (argc < 5) {
		fprintf(stderr, "Missing arguments");
		return -1;
	}
	module = argv[1];
	test = argv[2];
	host = argv[3];
	port = atoi(argv[4]);
	result = -1;
	if (test_setup(host, port) == 0) {
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
		} else if (strcmp(module, "misc") == 0) {
			result = test_misc(test);
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
		if (!riack_connect(test_client, host, port)) {
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



