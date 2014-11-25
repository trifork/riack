/*
   Copyright 2013 Trifork A/S
   Author: Kaspar Bach Pedersen

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma warning( disable:4005 )
#define _CRT_SECURE_NO_WARNINGS

#include "riack.h"
#include "riack_internal.h"
#include "riack_helpers.h"
#include <string.h>


void riack_free_copied_rpb_search_req(riack_client *client,
                                      RpbSearchQueryReq* search_req)
{
    if (search_req->has_df) {
        RFREE(client, search_req->df.data);
    }
    if (search_req->has_filter) {
        RFREE(client, search_req->filter.data);
    }
    if (search_req->has_op) {
        RFREE(client, search_req->op.data);
    }
    if (search_req->has_presort) {
        RFREE(client, search_req->presort.data);
    }
    if (search_req->has_sort) {
        RFREE(client, search_req->sort.data);
    }
    if (search_req->n_fl > 0) {
        size_t i;
        for (i=0; i<search_req->n_fl; ++i) {
            RFREE(client, search_req->fl[i].data);
        }
        RFREE(client, search_req->fl);
    }
}

/// Copies all optional search parameters to the protobuffers request.
void riack_set_optional_search_properties_on_req(riack_client *client, riack_search_optional_params* props,
        RpbSearchQueryReq* search_req)
{
    size_t cnt;
    if (!props) {
        return;
    }
    if (props->default_field_present) {
        search_req->has_df = 1;
        RMALLOCCOPY(client,
                    search_req->df.data, search_req->df.len,
                    props->default_field.value, props->default_field.len);
    }
    if (props->default_operation_present) {
        search_req->has_op = 1;
        RMALLOCCOPY(client,
                    search_req->op.data, search_req->op.len,
                    props->default_operation.value, props->default_operation.len);
    }
    if (props->filter_present) {
        search_req->has_filter = 1;
        RMALLOCCOPY(client,
                    search_req->filter.data, search_req->filter.len,
                    props->filter.value, props->filter.len);
    }
    if (props->presort_present) {
        search_req->has_presort = 1;
        RMALLOCCOPY(client,
                    search_req->presort.data, search_req->presort.len,
                    props->presort.value, props->presort.len);
    }
    if (props->rowlimit_present) {
        search_req->has_rows = 1;
        search_req->rows = props->rowlimit;
    }
    if (props->sort_present) {
        search_req->has_sort = 1;
        RMALLOCCOPY(client,
                    search_req->sort.data, search_req->sort.len,
                    props->sort.value, props->sort.len);
    }
    if (props->start_present) {
        search_req->has_start = 1;
        search_req->start = props->start;
    }
    cnt = props->field_limits_count;
    search_req->n_fl = cnt;
    if (cnt > 0) {
        size_t i;
        search_req->fl = RMALLOC(client, sizeof(ProtobufCBinaryData) * cnt);
        for (i = 0; i<cnt; ++i) {
            RMALLOCCOPY(client,
                        search_req->fl[i].data, search_req->fl[i].len,
                        props->field_limits[i].value, props->field_limits[i].len);
        }
    }
}

void riack_copy_rpbsearchdoc_to_searchdoc(riack_client *client, RpbSearchDoc* rpbsearchdoc,
        riack_search_doc *searchdoc)
{
    size_t cnt, i;
    cnt = rpbsearchdoc->n_fields;
    searchdoc->field_count = cnt;
    if (cnt > 0) {
        searchdoc->fields = RMALLOC(client, sizeof(riack_pair)*cnt);
        for (i=0; i<cnt; ++i) {
            riack_copy_rpbpair_to_pair(client, rpbsearchdoc->fields[i], &searchdoc->fields[i]);
        }
    }
}

void riack_set_search_result_from_response(riack_client *client, RpbSearchQueryResp *response,
        riack_search_result** search_result)
{
    *search_result = riack_search_result_alloc(client);
    if (response->has_max_score) {
        (*search_result)->max_score_present = 1;
        (*search_result)->max_score = response->max_score;
    }
    if (response->has_num_found) {
        (*search_result)->num_found_present = 1;
        (*search_result)->num_found = response->num_found;
    }
    (*search_result)->document_count = response->n_docs;
    if (response->n_docs > 0) {
        size_t cnt, i;
        cnt = response->n_docs;
        (*search_result)->documents = (riack_search_doc*)RMALLOC(client, sizeof(riack_search_doc) * cnt);
        for (i=0; i<cnt; ++i) {
            riack_copy_rpbsearchdoc_to_searchdoc(client, response->docs[i], &(*search_result)->documents[i]);
        }
    }
}

void riack_free_search_document(riack_client* client, riack_search_doc* search_doc)
{
    size_t cnt = search_doc->field_count;
    if (cnt > 0) {
        size_t i;
        for (i=0; i<cnt; ++i) {
            riack_free_copied_pair(client, &search_doc->fields[i]);
        }
        RFREE(client, search_doc->fields);
    }
}

void riack_free_search_result_p(riack_client* client, riack_search_result** search_result)
{
    if (search_result && *search_result) {
        size_t cnt = (*search_result)->document_count;
        if (cnt > 0) {
            size_t i;
            for (i = 0; i < cnt; ++i) {
                riack_free_search_document(client, &(*search_result)->documents[i]);
            }
            RFREE(client, (*search_result)->documents);
        }
        RFREE(client, *search_result);
        *search_result = 0;
    }
}

riack_search_result* riack_search_result_alloc(riack_client* client)
{
    return RCALLOC(client, sizeof(riack_search_result));
}

riack_cmd_cb_result riack_search_cb(riack_client *client, RpbSearchQueryResp *response,
        riack_search_result** search_result)
{
    riack_set_search_result_from_response(client, response, search_result);
    return RIACK_CMD_DONE;
}

int riack_search(riack_client *client, riack_string *query, riack_string *index,
        riack_search_optional_params* optional_parameters, riack_search_result** search_result)
{
    RpbSearchQueryReq search_req = RPB_SEARCH_QUERY_REQ__INIT;
    if (!client || !RSTR_HAS_CONTENT_P(query) || !RSTR_HAS_CONTENT_P(index) || !search_result) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    search_req.q.data = (uint8_t*)query->value;
    search_req.q.len = query->len;
    search_req.index.data = (uint8_t*)index->value;
    search_req.index.len = index->len;
    riack_set_optional_search_properties_on_req(client, optional_parameters, &search_req);
    return riack_perform_commmand(client, &cmd_search, (struct rpb_base_req const *) &search_req,
            (cmd_response_cb) riack_search_cb, (void **) search_result);
}
