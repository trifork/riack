
#include "test.h"

#define CNT_KEY "crdt_counter_test"

int test_crdt(char* testcase)
{
    if (strcmp(testcase, "counter") == 0) {
        return test_counter();
    } else {
        return -1;
    }
}

int test_counter()
{
    riack_counter_update_properties update_props;
    riack_counter_get_properties get_props;
    riack_string key, bucket;
    int64_t value;
    int res;
    memset(&update_props, 0, sizeof(update_props));
    memset(&get_props, 0, sizeof(get_props));
    bucket.len = strlen(RIAK_TEST_BUCKET);
    bucket.value = RIAK_TEST_BUCKET;
    key.len = strlen(CNT_KEY);
    key.value = CNT_KEY;

    get_props.basic_quorum = 1;
    get_props.basic_quorum_use = 1;
    get_props.notfound_ok_use = 1;
    get_props.notfound_ok = 0;
    get_props.pr_use = 1;
    get_props.pr = 1;
    get_props.r_use = 1;
    get_props.r = 1;

    update_props.dw_use = 1;
    update_props.dw = 1;
    update_props.pw_use = 1;
    update_props.pw = 1;
    update_props.w_use = 1;
    update_props.w = 1;
    res = riack_counter_get(test_client, &bucket, &key, &get_props, &value);
    if (res == RIACK_SUCCESS) {
        int64_t readden_value;
        // test increment with properties, no returned value
        res = riack_counter_increment(test_client, &bucket, &key, 3, &update_props, 0);
        if (res == RIACK_SUCCESS) {
            // Get without props
            res = riack_counter_get(test_client, &bucket, &key, 0, &readden_value);
            if (res == RIACK_SUCCESS && readden_value == value + 3) {
                // Increment no properties return value
                res = riack_counter_increment(test_client, &bucket, &key, -5, 0, &readden_value);
                if (res == RIACK_SUCCESS && readden_value == value - 2) {
                    return 0;
                }
            }
        }
    } else {
        printf("Error: %s", test_client->last_error);
    }
    return 1;
}
