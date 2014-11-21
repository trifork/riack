
#include "test.h"

int test_meta_links(char* testcase)
{
	if (strcmp(testcase, "load") == 0) {
		return test_meta_links_load();
	} else if (strcmp(testcase, "metas") == 0) {
		return test_meta_links_metas();
	} else if (strcmp(testcase, "links") == 0) {
		return test_meta_links_metas();
	} else {
		return -1;
	}
}

riack_string_linked_list *test_make_links(riack_string_linked_list *keys_answers, riack_content *content)
{
    riack_string_linked_list *current;
	current = keys_answers;
	if (current) {
		if (current->next) {
			content->link_count = 2;
		} else {
			content->link_count = 1;
		}
		content->links = malloc(sizeof(riack_link) * content->link_count);
		content->links[0].bucket = riack_copy_from_cstring(test_client, RIAK_TEST_BUCKET_ANSWERS);
		content->links[0].key = riack_copy_string(test_client, current->string);
		content->links[0].tag = riack_copy_from_cstring(test_client, "link1");
		if (current->next) {
			current = current->next;
			content->links[1].bucket = riack_copy_from_cstring(test_client, RIAK_TEST_BUCKET_ANSWERS);
			content->links[1].key = riack_copy_string(test_client, current->string);
			content->links[1].tag = riack_copy_from_cstring(test_client, "link2");
		}
		if (current->next) {
			current = current->next;
		}
	}

	return current;
}

riack_string copy_string(riack_string *str) {
    riack_string result;
	result.len = str->len;
	result.value = 0;
	if (str->len>0) {
		result.value = malloc(str->len);
		memcpy(result.value, str->value, str->len);
	}
	return result;
}

riack_pair* copy_metas(riack_pair* pairs, size_t count) {
	size_t i;
    riack_pair* result = malloc(sizeof(riack_pair) * count);
	for (i=0; i<count; ++i) {
		result[i].key = copy_string(&(pairs[i].key));
		result[i].value_present = pairs[i].value_present;
		result[i].value_len = pairs[i].value_len;
		if (pairs[i].value_len > 0) {
			result[i].value = malloc(pairs[i].value_len);
			memcpy(result[i].value, pairs[i].value, pairs[i].value_len);
		}
	}
	return result;
}

riack_content *copy_content(riack_content *org) {
    riack_content *result = malloc(sizeof(riack_content));
	memset(result, 0, sizeof(riack_content));
	result->charset = copy_string(&org->charset);
	result->content_encoding = copy_string(&org->content_encoding);
	result->content_type = copy_string(&org->content_type);
	result->data_len = org->data_len;
	if (org->data_len > 0) {
		result->data = malloc(org->data_len);
		memcpy(result->data, org->data, org->data_len);
	}
	result->usermeta_count = org->usermeta_count;
	if (result->usermeta_count > 0) {
		result->usermetas = copy_metas(org->usermetas, org->usermeta_count);
	}
	result->index_count = org->index_count;
	if (org->index_count > 0) {
		result->indexes = copy_metas(org->indexes, org->index_count);
	}
	return result;
}

int test_meta_links_load()
{
	riack_string bucket_posts, bucket_answers;
	riack_get_object *get_post;
	riack_object put_post;
	riack_string_linked_list *keys_posts, *current_post, *keys_answers, *current_answer;
	bucket_posts.len = strlen(RIAK_TEST_BUCKET_POSTS);
	bucket_posts.value = RIAK_TEST_BUCKET_POSTS;
	bucket_answers.len = strlen(RIAK_TEST_BUCKET_ANSWERS);
	bucket_answers.value = RIAK_TEST_BUCKET_ANSWERS;
	// Make random links between posts and comments
	if ((riack_list_keys(test_client, &bucket_posts, &keys_posts) == RIACK_SUCCESS)&&
		(riack_list_keys(test_client, &bucket_answers, &keys_answers) == RIACK_SUCCESS)) {
		current_post = keys_posts;
		current_answer = keys_answers;
		while (current_post && current_post->next) {
			if (riack_get(test_client, &bucket_posts, &(current_post->string), 0, &get_post) == RIACK_SUCCESS) {
				if (get_post->object.content_count == 1) {
					memset(&put_post, 0, sizeof(put_post));

					put_post.bucket = copy_string(&bucket_posts);
					put_post.key = copy_string(&current_post->string);

					put_post.content_count = 1;
					put_post.content = copy_content(get_post->object.content);

					current_answer = test_make_links(current_answer, get_post->object.content);
					if (riack_put(test_client, &put_post, 0, 0) != RIACK_SUCCESS) {
						return 1;
					}
                    riack_free_object(test_client, &put_post);
				}
                riack_free_get_object_p(test_client, &get_post);
			}
			current_post = current_post->next;
		}
	}
	return 0;
}

int test_meta_links_metas()
{
	return 0;
}

int test_meta_links_links()
{
	return 0;
}
