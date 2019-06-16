#include "buffercache.h"
#include "renderer.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

// An array of buffer caches.
static bufcache_t buffers[NUM_BUF_INDICES];

// -------------------------------------------------------------------------------------------------

void bufcache_initialize(void)
{
	// Initialize buffer objects.
	for (int i = 0; i < NUM_BUF_INDICES; i++) {

		uint32_t vertex_size;

		switch (i) {
			
			case BUFIDX_PARTICLE:
				vertex_size = sizeof(vertex_particle_t);
				break;

			case BUFIDX_UI:
				vertex_size = sizeof(vertex_ui_t);
				break;

			default:
				vertex_size = sizeof(vertex_t);
				break;
		}

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

bufcache_t *bufcache_get(buffer_index_t index)
{
	if (index < NUM_BUF_INDICES) {
		return &buffers[index];
	}

	return NULL;
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

void bufcache_clear_all_vertices(buffer_index_t index)
{
	if (index < NUM_BUF_INDICES) {
		buffer_clear(&buffers[index].vertex_buffer);
	}
}

void bufcache_clear_all_indices(buffer_index_t index)
{
	if (index < NUM_BUF_INDICES) {
		buffer_clear(&buffers[index].index_buffer);
	}
}

vertexbuffer_t *bufcache_legacy_alloc_buffer(void *data, size_t num_elements, size_t elem_size,
                                             bool is_index_data, bool is_static_data)
{
	// Create a GPU buffer object.
	vbindex_t vbo = rend_generate_buffer();

	// Create a vertexbuffer object to store buffer data into.
	NEW(vertexbuffer_t, buffer);
	buffer->vbo = vbo;
	buffer->count = num_elements;

	// Upload the vertex/index data to the GPU.
	rend_upload_buffer_data(buffer->vbo, data, num_elements * elem_size,
                            is_index_data, is_static_data);

	return buffer;
}

void bufcache_legacy_upload_buffer(vertexbuffer_t *buffer, void *data, size_t num_elements,
                                   size_t elem_size, bool is_index_data, bool is_static_data)
{
	if (buffer == NULL) {
		return;
	}
	
	buffer->count = num_elements;

	// Upload the vertex/index data to the GPU.
	rend_upload_buffer_data(buffer->vbo, data, num_elements * elem_size,
                            is_index_data, is_static_data);

}

void bufcache_legacy_destroy_buffer(vertexbuffer_t *buffer)
{
	if (buffer == NULL) {
		return;
	}

	if (buffer->vbo != 0) {
		rend_destroy_buffer(buffer->vbo);
	}

	DESTROY(buffer);
}
