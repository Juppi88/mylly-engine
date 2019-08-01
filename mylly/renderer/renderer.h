#pragma once
#ifndef __RENDERER_H
#define __RENDERER_H

// -------------------------------------------------------------------------------------------------

#include "core/defines.h"
#include "renderer/renderview.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

bool rend_initialize(void);
void rend_shutdown(void);

void rend_begin_draw(void);
void rend_end_draw(void);

void rend_draw_views(rview_t *first_view);

//
// Vertex buffer objects
//

// Generate a new vertex buffer object. Will be freed automatically after a while unless
// explicitly refreshed.
vbindex_t rend_generate_buffer(void);

// Destroy a generated vertex buffer object. Should only be called on renderer shutdown.
void rend_destroy_buffer(vbindex_t vbo);

// Upload vertex/index data to the GPU. Size of the buffer is the number of bytes to be uploaded.
void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index, bool is_static);

// Update vertex/index data in a vertex buffer object.
void rend_update_buffer_subdata(vbindex_t vbo, const void *data, size_t offset, size_t size,
	                            bool is_index);

//
// Shaders/shader programs
//

// Compile a shader from an array of source code lines. Returns the shader object if the compile
// succeeds, 0 otherwise. Will save the compiler log to compiler_log if set (can be left to NULL).
shader_object_t rend_create_shader(SHADER_TYPE type, const char **lines, size_t num_lines,
								   const char **compiler_log);

// Links a list of shader objects into a shader program. Returns the shader program if succeeds,
// 0 otherwise.
shader_program_t rend_create_shader_program(shader_object_t *shaders, size_t num_shaders);

// Destroy a previously compiled shader object.
void rend_destroy_shader(shader_object_t shader);

// Destroy a shader program.
void rend_destroy_shader_program(shader_program_t program);

// Returns the location of a uniform in a shader program, -1 if the uniform is not declared.
int rend_get_program_uniform_location(shader_program_t program, const char *name);

// Returns the index of a vertex attribute in a shader program, -1 if the attribute is not declared.
int rend_get_program_program_attribute_location(shader_program_t program, const char *name);

// Returns the renderer dependent source code for several built in default shaders.
typedef enum {

	DEFAULT_SHADER_MAIN,
	DEFAULT_SHADER_SPLASHSCREEN,
	DEFAULT_SHADER_DRAW_FRAMEBUFFER

} default_shader_t;

const char *rend_get_default_shader_source(default_shader_t shader);

//
// Textures
//

// Generate a GPU texture object.
texture_name_t rend_generate_texture(void *image, size_t width, size_t height,
                                     TEX_FORMAT fmt, TEX_FILTER filter);

// Destroy a texture on the GPU.
void rend_delete_texture(texture_name_t texture);

// Draw the splash screen while the engine is loading.
void rend_draw_splash_screen(texture_t *texture, shader_t *shader, colour_t background);

//
// Debug functionality
//
typedef enum gbuffer_component_t {

	GBUFFER_NONE,
	GBUFFER_COLOUR,
	GBUFFER_NORMAL,
	GBUFFER_DEPTH,
	NUM_GBUFFER_COMPONENTS

} gbuffer_component_t;

// Override current rendering and draw contents of the g-buffer on the screen.
void rend_override_draw_gbuffer(gbuffer_component_t buffer);

END_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

#endif
