#pragma once
#ifndef __OPENGL_FRAMEBUFFER_H
#define __OPENGL_FRAMEBUFFER_H

#include "renderer/opengl/opengl.h"
#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef struct gl_framebuffer_t {

	GLuint buffer; // Framebuffer object

	GLuint colour; // G-buffer attachments
	GLuint normal;
	GLuint depth;

} gl_framebuffer_t;

// -------------------------------------------------------------------------------------------------

bool rend_fb_initialize(void);
void rend_fb_shutdown(void);

void rend_bind_fb(int index);
void rend_clear_fbs(void);

const gl_framebuffer_t *rend_get_fb(int index);

#endif
