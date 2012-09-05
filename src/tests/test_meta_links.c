
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

struct RIACK_STRING_LINKED_LIST *test_make_links(struct RIACK_STRING_LINKED_LIST *keys_answers, struct RIACK_CONTENT *content)
{
	struct RIACK_STRING_LINKED_LIST *current;
	current = keys_answers;
	if (current) {
		if (current->next) {
			content->link_count = 2;
		} else {
			content->link_count = 1;
		}
		content->links = malloc(sizeof(struct RIACK_LINK) * content->link_count);
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

int test_meta_links_load()
{
	RIACK_STRING bucket_posts, bucket_answers;
	struct RIACK_GET_OBJECT get_post;
	struct RIACK_OBJECT put_post;
	struct RIACK_STRING_LINKED_LIST *keys_posts, *current_post, *keys_answers, *current_answer;
	bucket_posts.len = strlen(RIAK_TEST_BUCKET_POSTS);
	bucket_posts.value = RIAK_TEST_BUCKET_POSTS;
	bucket_answers.len = strlen(RIAK_TEST_BUCKET_ANSWERS);
	bucket_answers.value = RIAK_TEST_BUCKET_ANSWERS;
	// Make random links between posts and comments
	if ((riack_list_keys(test_client, bucket_posts, &keys_posts) == RIACK_SUCCESS)&&
		(riack_list_keys(test_client, bucket_answers, &keys_answers) == RIACK_SUCCESS)) {
		current_post = keys_posts;
		current_answer = keys_answers;
		while (current_post->next) {
			if (riack_get(test_client, bucket_posts, current_post->string, 0, &get_post) == RIACK_SUCCESS) {
				if (get_post.object.content_count == 1) {
					memset(&put_post, 0, sizeof(put_post));

					put_post.bucket = bucket_posts;
					put_post.key = current_post->string;

					// TODO Don't do this, since we are freeing the get_post
					put_post.content_count = get_post.object.content_count;
					put_post.content = get_post.object.content;

					current_answer = test_make_links(current_answer, get_post.object.content);
					if (riack_put(test_client, put_post, 0, 0) != RIACK_SUCCESS) {
						return 1;
					}
				}
				riack_free_get_object(test_client, &get_post);
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
