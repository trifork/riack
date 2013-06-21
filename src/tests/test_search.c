
#include "test.h"

#define SEARCH_BUCKET1 "testsearch"

int test_search(char* testcase)
{
    if (strcmp(testcase, "search1") == 0) {
        return test_search1();
    } else {
        return -1;
    }
}

int test_search1()
{
    int result;
    RIACK_STRING test_query;
    test_query.value = "";
    test_query.len = strlen(test_query.value);
    //riack_search(test_client, )
    return 0;
}
