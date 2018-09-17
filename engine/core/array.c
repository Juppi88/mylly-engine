#include "array.h"
#include "memory.h"
#include <string.h>

void arr_resize(void **arr, size_t *count, size_t *capacity, size_t data_size)
{
	if (*capacity >= data_size * ((*count) + 1)) {

		// Enough space for one more element, no need to resize.
		return;
	}

	// Create the new array.
	size_t new_capacity = (*capacity == 0 ? INITIAL_CAPACITY : (*capacity) << 1);
	void *new_arr = mem_alloc_fast(new_capacity * data_size);

	// Copy entries from the old array.
	if (*arr && *count != 0) {
		memcpy(new_arr, *arr, (*count) * data_size);
	}

	// Delete the old array.
	DELETE(*arr);

	*arr = new_arr;
	*capacity = new_capacity;
}

void arr_splice(void **arr, size_t *count, size_t *capacity, size_t data_size, int start, int items)
{
	memmove(*arr + start * data_size, *arr + (start + items) * data_size,
			(*count - start - items) * data_size);

	*count = *count - items;
}
