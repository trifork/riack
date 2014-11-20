/*
   Copyright 2012 Trifork A/S
   Author: Kaspar Pedersen

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

#include "riack_helpers.h"
#include "riack.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void riack_dbg_print_mapred_result(RIACK_MAPRED_RESPONSE_LIST *mapred) {
    RIACK_MAPRED_RESPONSE_LIST *current;
	char buffer[4000];
	current = mapred;
	while (current) {
        printf("     Phase: %u\n", current->response.phase);
        if (current->response.data_size > 0) {
            memcpy(buffer, current->response.data, current->response.data_size);
            buffer[current->response.data_size] = 0;
			printf("     Data:\n%s\n", buffer);
		}
		current = current->next_result;
	}
}

RIACK_GET_OBJECT* riack_get_object_alloc(RIACK_CLIENT* client)
{
    return RCALLOC(client, sizeof(RIACK_GET_OBJECT));
}

RIACK_OBJECT* riack_object_alloc(RIACK_CLIENT* client)
{
    return RCALLOC(client, sizeof(RIACK_OBJECT));
}

RIACK_STRING_LIST* riack_string_list_alloc(RIACK_CLIENT* client)
{
    return RCALLOC(client, sizeof(RIACK_STRING_LIST));
}

RIACK_STRING* riack_string_alloc(RIACK_CLIENT* client) {
    return RCALLOC(client, sizeof(RIACK_STRING));
}

/**
 * Free all memory associated with a RpbPair
 */
void riack_free_copied_rpb_pair(RIACK_CLIENT* client, RpbPair* ppair)
{
	RFREE(client, ppair->key.data);
	if (ppair->has_value) {
		RFREE(client, ppair->value.data);
	}
}

/**
 * Free all memory associated with a RpbLink
 */
void riack_free_copied_rpb_link(RIACK_CLIENT* client, RpbLink* plink)
{
	RFREE(client, plink->bucket.data);
	RFREE(client, plink->key.data);
	RFREE(client, plink->tag.data);
}

/**
 * Free all memory associated with this RpbContent
 */
void riack_free_copied_rpb_content(RIACK_CLIENT* client, RpbContent* pcontent)
{
	size_t n, i;

	RFREE(client, pcontent->charset.data);
	RFREE(client, pcontent->content_encoding.data);
	RFREE(client, pcontent->content_type.data);
	RFREE(client, pcontent->vtag.data);
	RFREE(client, pcontent->value.data);
	n = pcontent->n_indexes;
	if (n > 0) {
		for (i=0; i<n; ++i) {
            riack_free_copied_rpb_pair(client, pcontent->indexes[i]);
			RFREE(client, pcontent->indexes[i]);
		}
		RFREE(client, pcontent->indexes);
	}
	n = pcontent->n_usermeta;
	if (n > 0) {
		for (i=0; i<n; ++i) {
            riack_free_copied_rpb_pair(client, pcontent->usermeta[i]);
			RFREE(client, pcontent->usermeta[i]);
		}
		RFREE(client, pcontent->usermeta);
	}
	n = pcontent->n_links;
	if (n > 0) {
		for (i=0; i<n; ++i) {
            riack_free_copied_rpb_link(client, pcontent->links[i]);
			RFREE(client, pcontent->links[i]);
		}
		RFREE(client, pcontent->links);
	}
}

/**
 * Free alle memory associated with a RpbPutReq
 */
void riack_free_copied_rpb_put_req(RIACK_CLIENT* client, RpbPutReq* pput_req)
{
	if (pput_req) {
		RFREE(client, pput_req->bucket.data);
		RFREE(client, pput_req->vclock.data);
		RFREE(client, pput_req->key.data);
        riack_free_copied_rpb_content(client, pput_req->content);
        RFREE(client, pput_req->content);
    }
}

void riack_free_copied_pair(RIACK_CLIENT* client, RIACK_PAIR *ppair)
{
	RFREE(client, ppair->key.value);
	if (ppair->value_present) {
		RFREE(client, ppair->value);
	}
}

void riack_free_copied_link(RIACK_CLIENT* client, RIACK_LINK *plink)
{
	RFREE(client, plink->bucket.value);
	RFREE(client, plink->key.value);
	RFREE(client, plink->tag.value);
}

void riack_free_content(RIACK_CLIENT* client, RIACK_CONTENT *pcontent)
{
	size_t cnt, i;
	RFREE(client, pcontent->charset.value);
	RFREE(client, pcontent->content_encoding.value);
	RFREE(client, pcontent->content_type.value);
	RFREE(client, pcontent->vtag.value);
	if (pcontent->data_len > 0) {
		RFREE(client, pcontent->data);
	}
	cnt = pcontent->index_count;
	if (cnt > 0) {
		for (i=0; i<cnt; ++i) {
            riack_free_copied_pair(client, &pcontent->indexes[i]);
		}
		RFREE(client,pcontent->indexes);
	}

	cnt = pcontent->usermeta_count;
	if (cnt > 0) {
		for (i=0; i<cnt; ++i) {
            riack_free_copied_pair(client, &pcontent->usermetas[i]);
		}
		RFREE(client,pcontent->usermetas);
	}

	cnt = pcontent->link_count;
	if (cnt > 0) {
		for (i=0; i<cnt; ++i) {
            riack_free_copied_link(client, &pcontent->links[i]);
		}
		RFREE(client,pcontent->links);
	}
}

void riack_free_object(RIACK_CLIENT *client, RIACK_OBJECT *object)
{
    size_t cnt, i;
    if (object) {
        RFREE(client, object->bucket.value);
        RFREE(client, object->key.value);
        if (object->vclock.len > 0) {
            RFREE(client, object->vclock.clock);
        }
        cnt = object->content_count;
        if (cnt > 0) {
            for (i=0; i<cnt; ++i) {
                riack_free_content(client, &object->content[i]);
            }
            RFREE(client,object->content);
        }
    }
}

void riack_free_object_p(RIACK_CLIENT *client, RIACK_OBJECT **object)
{
	if (object && *object) {
        riack_free_object(client, *object);
        RFREE(client, *object);
        *object = 0;
	}
}

void riack_free_get_object_p(RIACK_CLIENT *client, RIACK_GET_OBJECT **object)
{
    if (object && *object) {
        riack_free_object(client, &(*object)->object);
        RFREE(client, *object);
        *object = 0;
    }
}

void riack_free_mapred_result(RIACK_CLIENT* client, RIACK_MAPRED_RESPONSE_LIST *result)
{
    RIACK_MAPRED_RESPONSE_LIST *current, *last;
	current = result;
	last = 0;
	while (current) {
        if (current->response.data_size > 0 && current->response.data) {
            RFREE(client, current->response.data);
		}
		last = current;
        current = current->next_result;
		RFREE(client, last);
	}
}

void riack_free_string_p(RIACK_CLIENT* client, RIACK_STRING** string)
{
    if (string != 0 && *string != 0) {
        riack_free_string(client, *string);
        RFREE(client, *string);
        *string = 0;
    }
}

void riack_free_string(RIACK_CLIENT* client, RIACK_STRING* string)
{
	RFREE(client, string->value);
	string->len = 0;
}

void riack_free_string_list_p(RIACK_CLIENT *client, RIACK_STRING_LIST **strings)
{
    size_t i;
    if (!strings) {
        return;
    }
    for (i=0; i<(*strings)->string_count; ++i) {
        riack_free_string(client, &((*strings)->strings[i]));
    }
    RFREE(client, (*strings)->strings);
    RFREE(client, *strings)
}


void riack_free_string_linked_list_p(RIACK_CLIENT *client, RIACK_STRING_LINKED_LIST **strings)
{
	 RIACK_STRING_LINKED_LIST *current, *next;
	 current = *strings;
	 while (current != 0) {
		 next = current->next;
		 riack_free_string(client, &current->string);
		 RFREE(client, current);
		 current = next;
	 }
	 *strings = 0;
}

RIACK_STRING riack_copy_string(RIACK_CLIENT* client, RIACK_STRING source)
{
	RIACK_STRING result;
	RMALLOCCOPY(client, result.value, result.len, source.value, source.len);
	return result;
}

RIACK_STRING riack_copy_from_cstring(RIACK_CLIENT* client, const char* source)
{
	RIACK_STRING result;
	RMALLOCCOPY(client, result.value, result.len, source, strlen(source));
	return result;
}

void riack_string_linked_list_set_entry(RIACK_CLIENT *client,
										RIACK_STRING_LINKED_LIST** entry,
										RIACK_STRING string_new)
{
	*entry = RMALLOC(client, sizeof(RIACK_STRING_LINKED_LIST));
	(*entry)->next = 0;
	(*entry)->string = string_new;
}

RIACK_STRING_LINKED_LIST* riack_string_linked_list_add(RIACK_CLIENT *client,
		RIACK_STRING_LINKED_LIST** base,
		RIACK_STRING string_new)
{
	RIACK_STRING_LINKED_LIST *current;
	current = *base;
	if (current == 0) {
		riack_string_linked_list_set_entry(client, base, string_new);
		current = *base;
	} else {
		while (current->next != 0) {
			current = current->next;
		}
		riack_string_linked_list_set_entry(client, &current->next, string_new);
	}
	return current;
}

void riack_mapred_add_to_chain(RIACK_CLIENT *client,
        RIACK_MAPRED_RESPONSE_LIST** base,
        RIACK_MAPRED_RESPONSE_LIST* mapred_new)
{
    RIACK_MAPRED_RESPONSE_LIST* current;
	if (*base == 0) {
		*base = mapred_new;
	} else {
		current = *base;
		while (current->next_result != 0) {
			current = current->next_result;
		}
		current->next_result = mapred_new;
	}
}

void riack_copy_buffer_to_string(RIACK_CLIENT* client, ProtobufCBinaryData* src, char** str)
{
	*str = RMALLOC(client, src->len + 1);
	memcpy(*str, src->data, src->len);
	(*str)[src->len] = 0;
}

/**
 * Copy a zero terminated string into a ProtobufCBinaryData structure
 */
void riack_copy_string_to_buffer(RIACK_CLIENT* client, char* str, ProtobufCBinaryData* target)
{
	target->len = strlen(str);
	target->data = (uint8_t*)RMALLOC(client, target->len);
	memcpy(target->data, str, target->len);
}

void riack_copy_rpblink_to_link(RIACK_CLIENT* client, RpbLink* src, RIACK_LINK* target)
{
	if (src->has_key) {
		RMALLOCCOPY(client, target->key.value, target->key.len, src->key.data, src->key.len);
	} else {
		target->key.value = 0;
		target->key.len = 0;
	}
	if (src->has_bucket) {
		RMALLOCCOPY(client, target->bucket.value, target->bucket.len, src->bucket.data, src->bucket.len);
	} else {
		target->bucket.value = 0;
		target->bucket.len = 0;
	}
	if (src->has_tag) {
		RMALLOCCOPY(client, target->tag.value, target->tag.len, src->tag.data, src->tag.len);
	} else {
		target->tag.value = 0;
		target->tag.len = 0;
	}
}

/**
 * Copy a RIACK_LINK structure to a RpbLink structure
 */
void riack_copy_link_to_rpblink(RIACK_CLIENT* client, RIACK_LINK* rlink, RpbLink* rpc_link)
{
	rpb_link__init(rpc_link);
	if (rlink->bucket.value) {
		rpc_link->has_bucket = 1;
		RMALLOCCOPY(client, rpc_link->bucket.data, rpc_link->bucket.len,
				rlink->bucket.value, rlink->bucket.len);
	}
	if (rlink->key.value) {
		rpc_link->has_key = 1;
		RMALLOCCOPY(client, rpc_link->key.data, rpc_link->key.len,
				rlink->key.value, rlink->key.len);
	}
	if (rlink->tag.value) {
		rpc_link->has_tag = 1;
		RMALLOCCOPY(client, rpc_link->tag.data, rpc_link->tag.len,
				rlink->tag.value, rlink->tag.len);
	}
}

/**
 * Copy a RIACK_PAIR structure to a RpbPair structure
 */
void riack_copy_pair_to_rpbpair(RIACK_CLIENT* client, RIACK_PAIR* rpair, RpbPair* rpc_pair)
{
	rpb_pair__init(rpc_pair);
	if (rpair->key.value) {
		RMALLOCCOPY(client, rpc_pair->key.data, rpc_pair->key.len, rpair->key.value, rpair->key.len);
	}
	rpc_pair->has_value = rpair->value_present;
	if (rpair->value_present) {
		rpc_pair->value.len = rpair->value_len;
		rpc_pair->value.data = (uint8_t*)RMALLOC(client, rpair->value_len);
		memcpy(rpc_pair->value.data, rpair->value, rpair->value_len);
	}
}

void riack_copy_rpbpair_to_pair(RIACK_CLIENT* client, RpbPair* rpc_pair, RIACK_PAIR* rpair)
{
	RMALLOCCOPY(client, rpair->key.value, rpair->key.len, rpc_pair->key.data, rpc_pair->key.len);
	rpair->value_present = (uint8_t) rpc_pair->has_value;
	if (rpair->value_present) {
		rpair->value_len = rpc_pair->value.len;
		rpair->value = (uint8_t*)RMALLOC(client, rpair->value_len);
		memcpy(rpair->value, rpc_pair->value.data, rpair->value_len);
	}
}

/**
 * Copy the content of a RIACK_CONTENT structure to a RpbContent structure
 * All memory and strings are copied
 */
void riack_copy_content_to_rpbcontent(RIACK_CLIENT* client, RIACK_CONTENT *pcontent, RpbContent* ppbc_content)
{
	size_t i;
	if (pcontent->charset.value) {
		ppbc_content->has_charset = 1;
		RMALLOCCOPY(client, ppbc_content->charset.data, ppbc_content->charset.len,
				pcontent->charset.value, pcontent->charset.len);
	}
	if (pcontent->content_encoding.value) {
		ppbc_content->has_content_encoding = 1;
		RMALLOCCOPY(client, ppbc_content->content_encoding.data, ppbc_content->content_encoding.len,
				pcontent->content_encoding.value, pcontent->content_encoding.len);
	}
	if (pcontent->content_type.value) {
		ppbc_content->has_content_type = 1;
		RMALLOCCOPY(client, ppbc_content->content_type.data, ppbc_content->content_type.len,
				pcontent->content_type.value, pcontent->content_type.len);
	}
	if (pcontent->vtag.value) {
		ppbc_content->has_vtag = 1;
		RMALLOCCOPY(client, ppbc_content->vtag.data, ppbc_content->vtag.len,
				pcontent->vtag.value, pcontent->vtag.len);
	}
	ppbc_content->value.len = pcontent->data_len;
	ppbc_content->value.data = (uint8_t*)RMALLOC(client, pcontent->data_len);
	memcpy(ppbc_content->value.data, pcontent->data, pcontent->data_len);

	ppbc_content->n_links = pcontent->link_count;
	if (pcontent->link_count > 0) {
		ppbc_content->links = (RpbLink**)RMALLOC(client, sizeof(RpbLink**) * pcontent->link_count);
		for (i=0; i<pcontent->link_count; ++i) {
			ppbc_content->links[i] = (RpbLink*)RMALLOC(client, sizeof(RpbLink));
            riack_copy_link_to_rpblink(client, &pcontent->links[i], ppbc_content->links[i]);
		}
	}

	ppbc_content->has_last_mod = pcontent->last_modified_present;
	ppbc_content->last_mod = pcontent->last_modified;
	ppbc_content->has_last_mod_usecs = pcontent->last_modified_usecs_present;
	ppbc_content->last_mod_usecs = pcontent->last_modified_usecs;
	ppbc_content->has_deleted = pcontent->deleted_present;
	ppbc_content->deleted = pcontent->deleted;

	ppbc_content->n_usermeta = pcontent->usermeta_count;
	if (pcontent->usermeta_count > 0) {
		ppbc_content->usermeta = (RpbPair**)RMALLOC(client, sizeof(RpbPair**) * pcontent->usermeta_count);
		for (i=0; i<pcontent->usermeta_count; ++i) {
			ppbc_content->usermeta[i] = (RpbPair*)RMALLOC(client, sizeof(RpbPair));
            riack_copy_pair_to_rpbpair(client, &pcontent->usermetas[i], ppbc_content->usermeta[i]);
		}
	}

	ppbc_content->n_indexes = pcontent->index_count;
	if (pcontent->index_count > 0) {
		ppbc_content->indexes = (RpbPair**)RMALLOC(client, sizeof(RpbPair**) * pcontent->index_count);
		for (i=0; i<pcontent->index_count; ++i) {
			ppbc_content->indexes[i] = (RpbPair*)RMALLOC(client, sizeof(RpbPair));
            riack_copy_pair_to_rpbpair(client, &pcontent->indexes[i], ppbc_content->indexes[i]);
		}
	}
}

/**
 * Copy contents of a RpbContent request into a RIACK_CONTENT structure.
 * Memory is copied so ppbc_content is ok to release after.
 */
void riack_copy_rpbcontent_to_content(RIACK_CLIENT* client, RpbContent *src, RIACK_CONTENT *target)
{
	size_t cnt, i;
	target->data_len = src->value.len;
	target->data = 0;
	if (target->data_len > 0) {
		target->data = (uint8_t*)RMALLOC(client, src->value.len);
		memcpy(target->data, src->value.data, src->value.len);
	}
	target->charset.value = 0;
	target->charset.len = 0;
	if (src->has_charset) {
		RMALLOCCOPY(client, target->charset.value, target->charset.len,
			src->charset.data, src->charset.len);
	}
	target->content_encoding.value = 0;
	target->content_encoding.len = 0;
	if (src->has_content_encoding) {
		RMALLOCCOPY(client, target->content_encoding.value, target->content_encoding.len,
			src->content_encoding.data, src->content_encoding.len);
	}
	target->content_type.value = 0;
	target->content_type.len = 0;
	if (src->has_content_type) {
		RMALLOCCOPY(client, target->content_type.value, target->content_type.len,
			src->content_type.data, src->content_type.len);
	}
	target->vtag.len = 0;
	target->vtag.value = 0;
	if (src->has_vtag) {
		RMALLOCCOPY(client, target->vtag.value, target->vtag.len,
			src->vtag.data, src->vtag.len);
	}

	cnt = src->n_indexes;
	target->index_count = cnt;
	if (cnt > 0) {
		target->indexes = (RIACK_PAIR*)RMALLOC(client, sizeof(RIACK_PAIR) * cnt);
		for (i=0; i<cnt; ++i) {
            riack_copy_rpbpair_to_pair(client, src->indexes[i], &target->indexes[i]);
		}
	}

	cnt = src->n_usermeta;
	target->usermeta_count = cnt;
	if (cnt > 0) {
		target->usermetas = (RIACK_PAIR*)RMALLOC(client, sizeof(RIACK_PAIR) * cnt);
		for (i=0; i<cnt; ++i) {
            riack_copy_rpbpair_to_pair(client, src->usermeta[i], &target->usermetas[i]);
		}
	}

	cnt = src->n_links;
	target->link_count = cnt;
	if (cnt > 0) {
		target->links = (RIACK_LINK*)RMALLOC(client, sizeof(RIACK_LINK) * cnt);
		for (i=0; i<cnt; ++i) {
            riack_copy_rpblink_to_link(client, src->links[i], &target->links[i]);
		}
	}

	target->last_modified_present = src->has_last_mod;
	target->last_modified = src->last_mod;
	target->last_modified_usecs_present = src->has_last_mod_usecs;
	target->last_modified_usecs = src->last_mod_usecs;
	target->deleted_present = src->has_deleted;
	target->deleted = src->deleted;
}

void riack_copy_object_to_rpbputreq(RIACK_CLIENT* client, RIACK_OBJECT *pobject, RpbPutReq* pput_req)
{
	RpbContent *content;
	content = (RpbContent*)RMALLOC(client, sizeof(RpbContent));
	rpb_content__init(content);

	riack_copy_content_to_rpbcontent(client, pobject->content, content);
	pput_req->content = content;
	pput_req->bucket.len = pobject->bucket.len;
	pput_req->bucket.data = (uint8_t*)RMALLOC(client, pobject->bucket.len);
	memcpy(pput_req->bucket.data, pobject->bucket.value, pobject->bucket.len);
	if (pobject->key.value) {
		pput_req->has_key = 1;
		pput_req->key.len = pobject->key.len;
		pput_req->key.data = (uint8_t*)RMALLOC(client, pobject->key.len);
		memcpy(pput_req->key.data, pobject->key.value, pobject->key.len);
	}
	if (pobject->vclock.len > 0) {
		pput_req->has_vclock = 1;
		pput_req->vclock.len = pobject->vclock.len;
		pput_req->vclock.data = (uint8_t*)RMALLOC(client, pobject->vclock.len);
		memcpy(pput_req->vclock.data, pobject->vclock.clock, pobject->vclock.len);
	}
}

void riack_link_strmapred_with_rpbmapred(RIACK_CLIENT* client, RpbMapRedResp* source,
                                         RIACK_MAPRED_RESPONSE* target)
{
	target->phase_present = (uint8_t) source->has_phase;
	target->phase = source->phase;
	if (source->has_response) {
		target->data_size = source->response.len;
		target->data = source->response.data;
	} else {
		target->data_size = 0;
		target->data = 0;
	}
}

void riack_copy_strmapred_to_mapred(RIACK_CLIENT* client, RIACK_MAPRED_RESPONSE* source,
                                    RIACK_MAPRED_RESPONSE_LIST* target)
{
	memset(target, 0, sizeof(*target));
    target->response.phase = source->phase;
    target->response.phase_present = source->phase_present;
	if (source->data_size > 0) {
        target->response.data_size = source->data_size;
        target->response.data = (uint8_t*)RMALLOC(client, source->data_size);
        memcpy(target->response.data, source->data, source->data_size);
	}
}

