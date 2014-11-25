
#include "test.h"

void *test_pb_alloc(void *allocator_data, size_t size);
void test_pb_free (void *allocator_data, void *data);

riack_allocator test_allocator =
{
	test_pb_alloc,
	test_pb_free,
};

void *test_pb_alloc(void *allocator_data, size_t size)
{
	void *rv;
	(void) allocator_data;
	if (size == 0) {
		return 0;
	}
	rv = malloc(size);
	return rv;
}

void test_pb_free (void *allocator_data, void *data)
{
	(void) allocator_data;
	if (data) {
		free(data);
	}
}
