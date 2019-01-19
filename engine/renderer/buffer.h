#pragma once
#ifndef __BUFFER_H
#define __BUFFER_H

#include "renderer/vertexbuffer.h"
#include "renderer/vertex.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

// Encodes buffer index, start offset and size into a single integer.
// 0-24: offset
// 24-48: size
// 48-52: buffer index (0-15)
typedef uint64_t buffer_handle_t;

#define MAX_BUFFER_SIZE 0xFFFFFF
#define BUFFER_OFFSET_MASK 0xFFFFFF
#define BUFFER_SIZE_SHIFT 24
#define BUFFER_SIZE_MASK 0xFFFFFF
#define BUFFER_INDEX_SHIFT 48
#define BUFFER_INDEX_MASK 0xF
#define BUFFER_TYPE_SHIFT 52

#define MAKE_BUFFER_HANDLE(offset, size, index, is_index_buffer)\
	((uint64_t)(offset) & BUFFER_OFFSET_MASK) |\
	(((uint64_t)(size) & BUFFER_SIZE_MASK) << BUFFER_SIZE_SHIFT) |\
	(((uint64_t)(index) & BUFFER_INDEX_MASK) << BUFFER_INDEX_SHIFT) |\
	(((uint64_t)(is_index_buffer) & 1) << BUFFER_TYPE_SHIFT)

#define BUFFER_GET_OFFSET(h) ((h) & BUFFER_SIZE_MASK)
#define BUFFER_GET_SIZE(h) (((h) >> BUFFER_SIZE_SHIFT) & BUFFER_SIZE_MASK)
#define BUFFER_GET_INDEX(h) (((h) >> BUFFER_INDEX_SHIFT) & BUFFER_INDEX_MASK)
#define BUFFER_IS_INDEX(h) (((h) >> BUFFER_INDEX_SHIFT) & BUFFER_INDEX_MASK)

// -------------------------------------------------------------------------------------------------

typedef enum buffer_type_t {

	BUFFER_INDEX,
	BUFFER_VERTEX,
	
} buffer_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct buffer_t {

	vbindex_t object; // Handle to GPU object
	buffer_type_t type; // Usage type of the buffer
	uint32_t index; // Index of the buffer
	uint32_t size; // Allocated size of the buffer
	uint32_t offset; // Current write offset

} buffer_t;

// -------------------------------------------------------------------------------------------------

void buffer_init(buffer_t *buffer, buffer_type_t usage, uint32_t index, uint32_t size);
void buffer_free(buffer_t *buffer);

buffer_handle_t buffer_alloc(buffer_t *buffer, const void *data, size_t data_size);
void buffer_update(buffer_t *buffer, buffer_handle_t handle, const void *data);

void buffer_clear(buffer_t *buffer);

END_DECLARATIONS;

#endif
