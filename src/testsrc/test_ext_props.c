
#include "test.h"

int test_ext_props(char* testcase)
{
    return test_ext_bucket_props();
}

int test_ext_bucket_props()
{
    RIACK_STRING bucket;
    RIACK_BUCKET_PROPERTIES props, *old_props, *read_props;

    memset(&props, 0, sizeof(RIACK_BUCKET_PROPERTIES));
    bucket.len = strlen(RIAK_TEST_BUCKET);
    bucket.value = RIAK_TEST_BUCKET;

    // Read properties so we can se them back afterwards.
    if (riack_get_bucket_props_ext(test_client, &bucket, 0, &old_props) != RIACK_SUCCESS) {
        return -1;
    }
    props.allow_mult_use = props.allow_mult = 1;
    props.basic_quorum_use = props.basic_quorum = 1;
    props.dw_use = 1;
    props.dw = 1;
    props.rw_use = 1;
    props.rw = 1;
    props.n_val_use = 1;
    props.n_val = 1;
    props.search_use = props.search = 1;

    if (riack_set_bucket_props(test_client, &bucket, &props) != RIACK_SUCCESS) {
        return -2;
    }
    if (riack_get_bucket_props_ext(test_client, &bucket, 0, &read_props) != RIACK_SUCCESS) {
        return -1;
    }
    if (!read_props->allow_mult_use ||  read_props->allow_mult != props.allow_mult) {
        return -3;
    }
    if (!read_props->basic_quorum_use ||  read_props->basic_quorum != props.basic_quorum) {
        return -4;
    }
    if (!read_props->dw_use ||  read_props->dw != props.dw) {
        return -5;
    }
    if (!read_props->rw_use ||  read_props->rw != props.rw) {
        return -6;
    }
    if (!read_props->n_val_use ||  read_props->n_val != props.n_val) {
        return -7;
    }
    if (!read_props->search_use ||  read_props->search != props.search) {
        return -8;
    }

    riack_free_bucket_properties_p(test_client, &read_props);
    riack_free_bucket_properties_p(test_client, &old_props);
    return 0;
}
