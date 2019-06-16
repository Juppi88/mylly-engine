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

// TODO: This is used for legacy methods, get rid of it.
typedef struct vertexbuffer_t {

	vbindex_t vbo; // Address/index of the generated GPU object
	size_t count; // Number of elements uploaded to the GPU

} vertexbuffer_t;

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

// TODO: These are remnants of the old vbcache system. In time the target is to get rid of these,
// but they're here for backwards compatibility with the old mesh system.
vertexbuffer_t *bufcache_legacy_alloc_buffer(void *data, size_t num_elements, size_t elem_size,
                                             bool is_index_data, bool is_static_data);

void bufcache_legacy_upload_buffer(vertexbuffer_t *buffer, void *data, size_t num_elements,
                                   size_t elem_size, bool is_index_data, bool is_static_data);

void bufcache_legacy_destroy_buffer(vertexbuffer_t *buffer);

END_DECLARATIONS;

#endif
