#pragma once
#ifndef __BUFFERCACHE_H
#define __BUFFERCACHE_H

#include "renderer/buffer.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

// TODO: This is an OpenGL ES 2.0 limitation, re-define for full OpenGL.
#define MAX_VERTICES 0xFFFF

// -------------------------------------------------------------------------------------------------

typedef enum buffer_index_t {

	BUFIDX_STATIC, // Static objects which persist in memory
	BUFIDX_DYNAMIC, // Dynamic objects with changing vertex data
	BUFIDX_PARTICLE, // Particle emitters
	BUFIDX_UI, // UI elements
	BUFIDX_DEBUG, // Debug elements (triangles)
	BUFIDX_DEBUG_LINE, // Debug elements (lines)
	BUFIDX_RESERVED1, // Reserved for future use
	BUFIDX_RESERVED2,
	BUFIDX_RESERVED3,
	BUFIDX_RESERVED4,
	BUFIDX_CUSTOM1, // Vertex buffers for custom meshes (i.e. tile grids)
	BUFIDX_CUSTOM2,
	BUFIDX_CUSTOM3,
	BUFIDX_CUSTOM4,
	BUFIDX_CUSTOM5,
	BUFIDX_CUSTOM6,

	NUM_BUF_INDICES
	
} buffer_index_t;

// -------------------------------------------------------------------------------------------------

typedef struct bufcache_t {

	buffer_t index_buffer;
	buffer_t vertex_buffer;

} bufcache_t;

// -------------------------------------------------------------------------------------------------

void bufcache_initialize(void);
void bufcache_shutdown(void);

bufcache_t *bufcache_get(buffer_index_t index);

// Allocate a buffer handle for a vertex array.
buffer_handle_t bufcache_alloc_vertices(buffer_index_t index, const void *data,
                                        uint32_t vertex_size, uint32_t num_elements);

// Allocate a buffer handle for an index array.
buffer_handle_t bufcache_alloc_indices(buffer_index_t index, const void *data,
	                                   uint32_t num_elements);

// Update buffer data. The size of the buffer must be the same as the one allocated initially.
void bufcache_update(buffer_handle_t handle, const void *data);

// Clear all dynamically built vertices or indices for rebuilding.
void bufcache_clear_all_vertices(buffer_index_t index);
void bufcache_clear_all_indices(buffer_index_t index);

END_DECLARATIONS;

#endif
