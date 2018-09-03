#include "memory.h"
#include "io/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *mem_alloc(size_t size)
{
	void *ptr = malloc(size);

	if (ptr == NULL) {
		log_error("Memory", "Unable to allocate memory!");
		exit(0);
	}

	memset(ptr, 0, size);
	return ptr;
}

void mem_free(void *ptr)
{
	free(ptr);
}
