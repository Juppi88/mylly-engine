#include "audiobuffer.h"
#include "audiosystem.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

static void audiobuffer_destroy(void *buffer_ptr);

// -------------------------------------------------------------------------------------------------

audiobuffer_t *audiobuffer_create(void)
{
	NEW(audiobuffer_t, buffer);

	ref_init(buffer, audiobuffer_destroy);

	// Create an OpenAL audio buffer object.
	buffer->buffer = audio_create_buffer();

	return buffer;
}

static void audiobuffer_destroy(void *buffer_ptr)
{
	if (buffer_ptr == NULL) {
		return;
	}

	audiobuffer_t *buffer = (audiobuffer_t *)buffer_ptr;

	// Destroy OpenAL buffer object.
	audio_destroy_buffer(buffer->buffer);

	mem_free(buffer_ptr);
}
