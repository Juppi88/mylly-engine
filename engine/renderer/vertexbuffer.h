#pragma once
#ifndef __VERTEXBUFFER_H
#define __VERTEXBUFFER_H

#include "core/defines.h"
#include "core/list.h"
#include "renderer/vertex.h"

// --------------------------------------------------------------------------------

typedef struct vertexbuffer_t {

	LIST_ENTRY(vertexbuffer_t);
	vertex_t *buffer;
	size_t count;

} vertexbuffer_t;

// --------------------------------------------------------------------------------

#define vertexbuffer() \
	{ NULL, NULL, 0 }

#endif
