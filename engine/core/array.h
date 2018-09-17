#pragma once
#ifndef __ARRAY_H
#define __ARRAY_H

#include "core/defines.h"
#include "core/memory.h"

#define INITIAL_CAPACITY 16
#define INVALID_INDEX 0xFFFFFFFF

#define arr_t(type) struct {\
	type *array;\
	size_t count;\
	size_t capacity;\
}

#define arr_init(arr)\
	(arr).array = NULL;\
	(arr).count = 0;\
	(arr).capacity = 0;

#define arr_clear(arr) {\
	DELETE((arr).array);\
	(arr).count = 0;\
	(arr).capacity = 0;\
}

#define arr_push(arr, item) {\
	if ((arr).count == (arr).capacity)\
		arr_resize((void **)&(arr).array, &(arr).count, &(arr).capacity, sizeof((arr).array[0]));\
	(arr).array[(arr).count++] = item;\
}

#define arr_foreach(arr, var)\
	for (size_t __i = 0; __i < (arr).count && ((var) = (arr).array[__i], 1); ++__i)


#define arr_find(arr, val, idx) {\
	for ((idx) = 0; (idx) < (arr).count; ++(idx)) {\
		if ((arr).array[(idx)] == (val)) break;\
	}\
	if ((idx) == (arr).count) (idx) = -1;\
}

#define arr_remove(arr, val) {\
	int __idx;\
    arr_find(arr, val, __idx);\
	if (__idx != -1) {\
		arr_splice((void **)&(arr).array, &(arr).count, &(arr).capacity,\
			sizeof((arr).array[0]), __idx, 1);\
	}\
}

// --------------------------------------------------------------------------------

void arr_resize(void **arr, size_t *count, size_t *capacity, size_t data_size);
void arr_splice(void **arr, size_t *count, size_t *capacity, size_t data_size, int start, int items);

#endif
