#include "buffercache.h"

// -------------------------------------------------------------------------------------------------

// An array of buffer caches.
static bufcache_t buffers[NUM_BUF_INDICES];

// -------------------------------------------------------------------------------------------------

void bufcache_initialize(void)
{
	// Initialize buffer objects.
	for (int i = 0; i < NUM_BUF_INDICES; i++) {

		uint32_t vertex_size = (i == BUFIDX_PARTICLE ? sizeof(vertex_particle_t) : sizeof(vertex_t));

		buffer_init(&buffers[i].index_buffer, BUFFER_INDEX, i, MAX_VERTICES * sizeof(vindex_t));
		buffer_init(&buffers[i].vertex_buffer, BUFFER_VERTEX, i, MAX_VERTICES * vertex_size);
	}
}

void bufcache_shutdown(void)
{
	// Deallocate buffer objects.
	for (int i = 0; i < NUM_BUF_INDICES; i++) {

		buffer_free(&buffers[i].index_buffer);
		buffer_free(&buffers[i].vertex_buffer);
	}
}

buffer_handle_t bufcache_alloc_vertices(buffer_index_t index, const void *data,
                                        uint32_t vertex_size, uint32_t num_elements)
{
	if (index >= NUM_BUF_INDICES) {
		return 0;
	}

	return buffer_alloc(&buffers[index].vertex_buffer, data, vertex_size * num_elements);
}

buffer_handle_t bufcache_alloc_indices(buffer_index_t index, const void *data,
	                                   uint32_t num_elements)
{
	if (index >= NUM_BUF_INDICES) {
		return 0;
	}

	return buffer_alloc(&buffers[index].index_buffer, data, sizeof(vindex_t) * num_elements);
}

void bufcache_update(buffer_handle_t handle, const void *data)
{
	if (handle == 0) {
		return;
	}

	// Get the index of the buffer cache from the buffer handle.
	int index = BUFFER_GET_INDEX(handle);

	// Update the data in the buffer.
	if (BUFFER_IS_INDEX(handle)) {
		buffer_update(&buffers[index].index_buffer, handle, data);
	}
	else {
		buffer_update(&buffers[index].vertex_buffer, handle, data);
	}
}
