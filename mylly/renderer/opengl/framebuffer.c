#include "framebuffer.h"
#include "extensions.h"
#include "renderer/vertex.h"
#include "renderer/renderer.h"
#include "core/mylly.h"

// -------------------------------------------------------------------------------------------------

static gl_framebuffer_t geometry_buffer; // Framebuffer used for the geometry pass
static gl_framebuffer_t deferred_buffers[2]; // Deferred/post processing framebuffers

// -------------------------------------------------------------------------------------------------

static bool rend_fb_create_buffer(gl_framebuffer_t *buffer, uint16_t width, uint16_t height,
                                  bool is_geometry);

// -------------------------------------------------------------------------------------------------

bool rend_fb_initialize(void)
{
	// Generate a screen sized texture for the frame buffer.
	uint16_t width, height;
	mylly_get_resolution(&width, &height);

	glActiveTexture(GL_TEXTURE0);

	// Geometry pass framebuffer.
	if (!rend_fb_create_buffer(&geometry_buffer, width, height, true)) {
		return false;
	}
	
	// Deferred/post processing framebuffers.
	for (int i = 0; i < 2; i++) {
		if (!rend_fb_create_buffer(&deferred_buffers[i], width, height, false)) {
			return false;
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void rend_fb_shutdown(void)
{
	glDeleteFramebuffers(1, &geometry_buffer.buffer);
	glDeleteTextures(1, &geometry_buffer.colour);
	glDeleteTextures(1, &geometry_buffer.normal);
	glDeleteTextures(1, &geometry_buffer.depth);
	glDeleteTextures(1, &geometry_buffer.specular);

	for (int i = 0; i < 2; i++) {

		glDeleteFramebuffers(1, &deferred_buffers[i].buffer);
		glDeleteTextures(1, &deferred_buffers[i].colour);
		glDeleteTextures(1, &deferred_buffers[i].normal);
		glDeleteTextures(1, &deferred_buffers[i].depth);
		glDeleteTextures(1, &deferred_buffers[i].specular);
	}
}

void rend_clear_fbs(void)
{
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, geometry_buffer.buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 2; i++) {

		glBindFramebuffer(GL_FRAMEBUFFER, deferred_buffers[i].buffer);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Clear hardware buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rend_bind_fb(int index)
{
	if (index >= 0 && index < 2) {
		glBindFramebuffer(GL_FRAMEBUFFER, deferred_buffers[index].buffer);
	}
	else if (index == FB_GEOMETRY) {
		glBindFramebuffer(GL_FRAMEBUFFER, geometry_buffer.buffer);
	}
	else {
		// Bind back to the screen's buffer.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

const gl_framebuffer_t *rend_get_fb(int index)
{
	if (index == FB_GEOMETRY) {
		return &geometry_buffer;
	}
	if (index >= 0 && index < 2) {
		return &deferred_buffers[index];
	}

	return NULL;
}

static bool rend_fb_create_buffer(gl_framebuffer_t *buffer, uint16_t width, uint16_t height,
                                  bool is_geometry)
{
	// Generate the framebuffer object.
	glGenFramebuffers(1, &buffer->buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->buffer);

	// Generate target textures and attach them to the framebuffer.
	glGenTextures(1, &buffer->colour);
	glBindTexture(GL_TEXTURE_2D, buffer->colour);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->colour, 0);

	if (is_geometry) {

		// Use multiple render targets with this framebuffer.
		unsigned int attachments[] = {
			GL_COLOR_ATTACHMENT0, // Colour/diffuse/albedo
			GL_COLOR_ATTACHMENT1, // Normal
			GL_COLOR_ATTACHMENT2  // Specular/shininess
		};

		glDrawBuffers(LENGTH(attachments), attachments);

		glGenTextures(1, &buffer->normal);
		glBindTexture(GL_TEXTURE_2D, buffer->normal);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->normal, 0);

		glGenTextures(1, &buffer->specular);
		glBindTexture(GL_TEXTURE_2D, buffer->specular);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer->specular, 0);

		glGenTextures(1, &buffer->depth);
		glBindTexture(GL_TEXTURE_2D, buffer->depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buffer->depth, 0);
	}

	// Check that the framebuffer was created successfully.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	return (status == GL_FRAMEBUFFER_COMPLETE);
}
