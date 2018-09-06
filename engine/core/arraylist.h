#pragma once
#ifndef __ARRAYLIST_H
#define __ARRAYLIST_H

#include "core/defines.h"
#include "core/memory.h"

#define GRANULARITY 16
#define INVALID_INDEX 0xFFFFFFFF

typedef struct {
	void *list;
	size_t count;
	size_t size;
} arraylist_t;

#define array_clear(arr) {\
	DELETE((arr).list);\
	(arr).count = 0;\
	(arr).size = 0;\
}

#define array_resize(type, arr, n) {\
	if (n == 0) {\
		array_clear(arr);\
	}\
	else if (n > (arr).count) {\
		type *__old = (arr).list;\
		NEW_ARRAY(type, __tmp, n);\
		for (size_t __i = 0; __i < (arr).count; ++__i) { __tmp[__i] = __old[__i]; }\
		(arr).list = __tmp;\
		(arr).size = n;\
		DELETE(__old);\
	}\
}

#define array_push(type, arr, item) {\
	if ((arr).count == (arr).size)\
		array_resize(type, (arr), (arr).size + GRANULARITY);\
	((type *)(arr).list)[(arr).count++] = item;\
}

#define array_get(type, arr, index, name)\
	type name = ((type *)(arr).list)[index]

#define array_foreach(type, arr, var)\
	for (size_t __idx = 0; __idx < (arr).count; ++__idx) {\
		type var = ((type *)(arr).list)[__idx];\


#define array_end() }

#endif
