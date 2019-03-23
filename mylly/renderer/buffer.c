#include "buffer.h"
#include "renderer.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

void buffer_init(buffer_t *buffer, buffer_type_t usage, uint32_t index, uint32_t size)
{
	if (buffer == NULL) {
		return;
	}

	// Ensure maximum buffer size isn't exceeded.
	if (size > MAX_BUFFER_SIZE) {

		log_warning("Renderer", "Attempting to allocate a too large buffer object, resizing.");
		size = MAX_BUFFER_SIZE;
	}

	// Initialize the buffer object.
	buffer->type = usage;
	buffer->index = index;
	buffer->size = size;
	buffer->offset = 0;

	// Generate the buffer object and allocate empty memory on the GPU.
	buffer->object = rend_generate_buffer();
	rend_upload_buffer_data(buffer->object, NULL, size, usage == BUFFER_INDEX, false);
}

void buffer_free(buffer_t *buffer)
{
	if (buffer == NULL) {
		return;
	}

	rend_destroy_buffer(buffer->object);
	buffer->object = 0;
}

buffer_handle_t buffer_alloc(buffer_t *buffer, const void *data, size_t size)
{
	if (buffer == NULL) {
		return 0;
	}

	// Ensure there is enough space in the buffer.
	if (buffer->size < buffer->offset + size) {

		log_error("Renderer", "Renderer buffer is full!");
		return 0;
	}

	// Create a new handle to a subrange of the vertex buffer object.
	uint32_t offset = buffer->offset;
	buffer_handle_t handle = MAKE_BUFFER_HANDLE(offset, size, buffer->index,
                                                buffer->type == BUFFER_INDEX);

	// Update the range in the buffer object.
	rend_update_buffer_subdata(buffer->object, data, offset, size, buffer->type == BUFFER_INDEX);

	// Advance buffer write offset by data size.
	buffer->offset += size;

	return handle;
}

void buffer_update(buffer_t *buffer, buffer_handle_t handle, const void *data)
{
	if (buffer == NULL) {
		return;
	}

	// Get data size and offset from the handle.
	uint32_t offset = BUFFER_GET_OFFSET(handle);
	uint32_t size = BUFFER_GET_SIZE(handle);

	// Update the data in the buffer object.
	rend_update_buffer_subdata(buffer->object, data, offset, size, buffer->type == BUFFER_INDEX);
}

void buffer_clear(buffer_t *buffer)
{
	if (buffer == NULL) {
		return;
	}

	buffer->offset = 0;
}
