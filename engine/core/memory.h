#pragma once
#ifndef __MEMORY_H
#define __MEMORY_H

#include "core/defines.h"

#define NEW(type, name) struct type *name = mem_alloc(sizeof(struct type))

#define NEW_ARRAY(type, name, count) type *name = mem_alloc_fast(sizeof(type) * count)

#define DELETE(var) {\
	mem_free(var);\
	(var) = NULL;\
}

BEGIN_DECLARATIONS;

void *mem_alloc(size_t size);
void *mem_alloc_fast(size_t size);
void mem_free(void *ptr);

END_DECLARATIONS;

#endif
