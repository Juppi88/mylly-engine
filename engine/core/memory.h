#pragma once
#ifndef __MEMORY_H
#define __MEMORY_H

#include "core/defines.h"

#define NEW(type, var) struct type *var = mem_alloc(sizeof(struct type))

void *mem_alloc(size_t size);
void mem_free(void *ptr);

#endif
