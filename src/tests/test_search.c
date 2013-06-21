
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
    RIACK_STRING test_query, index;
    struct RIACK_SEARCH_OPTIONAL_PARAMETERS params;
    struct RIACK_SEARCH_RESULT search_result;
    result = -2;
    // Load a lot of test data to search in
    process_file("testdata/c_friendly/comments.json.out", SEARCH_BUCKET1);
    memset(&params, 0, sizeof(struct RIACK_SEARCH_OPTIONAL_PARAMETERS));

    params.default_field_present = 1;
    params.default_field.value = "Body";
    params.default_field.len = strlen(params.default_field.value);

    test_query.value = "Twit*";
    test_query.len = strlen(test_query.value);
    index.value = SEARCH_BUCKET1;
    index.len = strlen(index.value);
    if (riack_search(test_client, test_query, index, &params, &search_result) == RIACK_SUCCESS) {
        result = 0;
    }
    // TODO Free result.
    test_load_cleanup_bucket(SEARCH_BUCKET1);
    return result;
}
