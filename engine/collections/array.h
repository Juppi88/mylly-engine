#pragma once
#ifndef __ARRAY_H
#define __ARRAY_H

/*
====================================================================================================

	Array

	A dynamic array type. Array info is not embedded into the collection members.

====================================================================================================
*/

#include "core/defines.h"
#include "core/memory.h"

#define INITIAL_CAPACITY 16
#define INVALID_INDEX 0xFFFFFFFF

#define arr_initializer { NULL, 0, 0 }

#define arr_t(type) struct {\
	type *items;\
	size_t count;\
	size_t capacity;\
}

#define arr_init(arr)\
	(arr).items = NULL;\
	(arr).count = 0;\
	(arr).capacity = 0;

#define arr_clear(arr) {\
	DESTROY((arr).items);\
	(arr).count = 0;\
	(arr).capacity = 0;\
}

#define arr_push(arr, item) {\
	if ((arr).count == (arr).capacity)\
		arr_resize((char **)&(arr).items, &(arr).count, &(arr).capacity, sizeof((arr).items[0]));\
	(arr).items[(arr).count++] = item;\
}

#define arr_set(arr, idx, val) \
	(arr).items[(idx)] = (val)

#define arr_foreach(arr, var)\
	for (size_t __i = 0; __i < (arr).count && ((var) = (arr).items[__i], 1); ++__i)

#define arr_foreach_reverse(arr, var)\
	for (size_t __i = (arr).count; __i > 0 && ((var) = (arr).items[__i - 1], 1); --__i)

#define arr_foreach_index() (__i)

#define arr_find(arr, val, idx) {\
	for ((idx) = 0; (idx) < (int)(arr).count; ++(idx)) {\
		if ((arr).items[(idx)] == (val)) break;\
	}\
	if ((idx) == (arr).count) (idx) = -1;\
}

#define arr_remove(arr, val) {\
	int __idx;\
	arr_find(arr, val, __idx);\
	if (__idx != -1) {\
		arr_splice((char **)&(arr).items, &(arr).count, &(arr).capacity,\
			sizeof((arr).items[0]), __idx, 1);\
	}\
}

#define arr_find_empty(arr, var) {\
	(var) = INVALID_INDEX;\
	for (size_t __i = 0; __i < (arr).count; ++__i) {\
		if ((arr).items[__i] == NULL) {\
			(var) = __i;\
			break;\
		}\
	}\
}

#define arr_last_index(arr)\
	((arr).count - 1)

// --------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void arr_resize(char **arr, size_t *count, size_t *capacity, size_t data_size);
void arr_splice(char **arr, size_t *count, size_t *capacity, size_t data_size, int start, int items);

END_DECLARATIONS;

#endif
