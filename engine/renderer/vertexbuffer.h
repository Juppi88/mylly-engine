#pragma once
#ifndef __VERTEXBUFFER_H
#define __VERTEXBUFFER_H

#include "core/defines.h"
#include "core/list.h"
#include "renderer/vertex.h"

// --------------------------------------------------------------------------------

typedef uint32_t vbindex_t;

typedef struct vertexbuffer_t {

	LIST_ENTRY(vertexbuffer_t);

	struct vertexbuffer_t **reference; // The object referencencing this buffer
	vbindex_t vbo; // Address/index of the generated GPU object
	size_t count; // Number of elements uploaded to the GPU
	int invalidates; // Frame count when this buffer is to be invalidated automatically.

} vertexbuffer_t;

// --------------------------------------------------------------------------------

#define vertexbuffer(vbo) \
	{ NULL, NULL, vbo, 0, 0 }

#endif
