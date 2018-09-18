#include "vbcache.h"
#include "renderer.h"

// Number of frames rendered.
static int frame_count;

static LIST(vertexbuffer_t) active_buffers;
static LIST(vertexbuffer_t) free_buffers;

// --------------------------------------------------------------------------------

static vertexbuffer_t *vbcache_find_free_buffer(void);
static void vbcache_destroy_all_buffers(void);

// --------------------------------------------------------------------------------

void vbcache_initialize(void)
{
}

void vbcache_shutdown(void)
{
	vbcache_free_inactive_buffers();
	vbcache_destroy_all_buffers();
}

void vbcache_set_current_frame(int frame)
{
	frame_count = frame;
}

void vbcache_alloc_buffer(void *data, size_t num_elements, size_t elem_size,
						  vertexbuffer_t **storage, bool is_index_data)
{
	vertexbuffer_t *buffer = vbcache_find_free_buffer();

	buffer->count = num_elements;
	buffer->reference = storage;
	*storage = buffer;

	// Upload the vertex/index data to the GPU.
	rend_upload_buffer_data(buffer->vbo, data, num_elements * elem_size, is_index_data);

	// Mark the buffer object as active.
	vbcache_refresh_buffer(buffer);
}

void vbcache_refresh_buffer(vertexbuffer_t *buffer)
{
	// Keep this vertex buffer in active use for at least the next two frames.
	buffer->invalidates = frame_count + VBCACHE_LIFE_TIME;
}

void vbcache_free_inactive_buffers(void)
{
	vertexbuffer_t *previous = NULL;

	// TODO: Use a real list here instead of a stack.
	// Stack makes removing from the middle of the stack messy.
	LIST_FOREACH_SAFE(vertexbuffer_t, buffer, active_buffers) {
		LIST_FOREACH_SAFE_BEGIN(buffer);

		// Check whether this vertex buffer has outlived its time.
		if (buffer->invalidates < frame_count) {

			// Rearrange the linked list references.
			if (LIST_IS_FIRST(active_buffers, buffer)) {
				active_buffers = LIST_FOREACH_SAFE_NEXT();
			}
			else if (previous != NULL) {
				previous->__next = LIST_FOREACH_SAFE_NEXT();
			}

			// Clear references to this buffer.
			if (buffer->reference != NULL) {
				*buffer->reference = NULL;
			}

			// Add this buffer to the free buffer list.
			LIST_ADD(free_buffers, buffer);
		}
		else {
			previous = buffer;
		}
	}
}

static void vbcache_destroy_all_buffers(void)
{
	LIST_FOREACH_SAFE(vertexbuffer_t, buffer, free_buffers) {
		LIST_FOREACH_SAFE_BEGIN(buffer);

		// Deallocate GPU resources.
		if (buffer->vbo != 0) {
			rend_destroy_buffer(buffer->vbo);
		}

		DELETE(buffer);
	}
}

static vertexbuffer_t *vbcache_find_free_buffer(void)
{
	// If there aren't any previously allocated free buffers available, create new ones.
	if (LIST_IS_EMPTY(free_buffers)) {

		for (int i = 0; i < VBCACHE_ADDITIONAL_BUFFERS; ++i) {

			vbindex_t vbo = rend_generate_buffer();

			NEW(vertexbuffer_t, buffer);
			buffer->vbo = vbo;

			// Add the new buffer to the list of free vertex buffers.
			LIST_ADD(free_buffers, buffer);
		}
	}

	// Return the first available buffer and move it to the active buffer list.
	vertexbuffer_t *available = LIST_POP_FIRST(free_buffers);
	LIST_ADD(active_buffers, available);

	return available;
}
