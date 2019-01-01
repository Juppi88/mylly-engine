#pragma once
#ifndef __MEMORY_H
#define __MEMORY_H

#include "core/defines.h"

#define NEW(type, name) struct type *name = (struct type *)mem_alloc(sizeof(struct type))

#define NEW_ARRAY(type, name, count) type *name = (type *)mem_alloc_fast(sizeof(type) * count)

#define DESTROY(var) {\
	mem_free((void *)(var));\
	(var) = NULL;\
}

BEGIN_DECLARATIONS;

void *mem_alloc(size_t size);
void *mem_alloc_fast(size_t size);
void mem_free(void *ptr);

END_DECLARATIONS;

#endif
