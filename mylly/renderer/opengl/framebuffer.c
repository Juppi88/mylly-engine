#include "framebuffer.h"
#include "extensions.h"
#include "renderer/vertex.h"
#include "renderer/renderer.h"
#include "core/mylly.h"

// -------------------------------------------------------------------------------------------------

static gl_framebuffer_t framebuffers[2];

// -------------------------------------------------------------------------------------------------

bool rend_fb_initialize(void)
{
	// Generate a screen sized texture for the frame buffer.
	uint16_t width, height;
	mylly_get_resolution(&width, &height);

	glActiveTexture(GL_TEXTURE0);
	
	for (int i = 0; i < 2; i++) {

		// Generate the framebuffer object.
		glGenFramebuffers(1, &framebuffers[i].buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i].buffer);

		// Generate target textures and attach them to the framebuffer.
		glGenTextures(1, &framebuffers[i].colour);
		glBindTexture(GL_TEXTURE_2D, framebuffers[i].colour);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffers[i].colour, 0);

		glGenTextures(1, &framebuffers[i].normal);
		glBindTexture(GL_TEXTURE_2D, framebuffers[i].normal);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffers[i].normal, 0);

		glGenTextures(1, &framebuffers[i].depth);
		glBindTexture(GL_TEXTURE_2D, framebuffers[i].depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffers[i].depth, 0);

		// Check that the framebuffer was created successfully.
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE) {
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
	for (int i = 0; i < 2; i++) {

		glDeleteFramebuffers(1, &framebuffers[i].buffer);
		glDeleteTextures(1, &framebuffers[i].colour);
		glDeleteTextures(1, &framebuffers[i].normal);
		glDeleteTextures(1, &framebuffers[i].depth);
	}
}

void rend_clear_fbs(void)
{
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);

	for (int i = 0; i < 2; i++) {

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i].buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Clear hardware buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rend_bind_fb(int index)
{
	if (index >= 0 && index < 2) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index].buffer);
	}
	else {
		// Bind back to the screen's buffer.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

const gl_framebuffer_t *rend_get_fb(int index)
{
	if (index < 0 || index >= 2) {
		return NULL;
	}

	return &framebuffers[index];
}
