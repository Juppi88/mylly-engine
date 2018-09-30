#pragma once
#ifndef __RENDERER_H
#define __RENDERER_H

#include "core/defines.h"
#include "renderer/model.h"
#include "renderer/renderview.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/vertexbuffer.h"

bool rend_initialize(void);
void rend_shutdown(void);

void rend_draw_views(LIST(rview_t) views);

//
// Vertex buffer objects
//

// Generate a new vertex buffer object. Will be freed automatically after a while unless
// explicitly refreshed.
vbindex_t rend_generate_buffer(void);

// Destroy a generated vertex buffer object. Should only be called on renderer shutdown.
void rend_destroy_buffer(vbindex_t vbo);

// Upload vertex/index data to the GPU. Size of the buffer is the number of elements.
void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index_data);

//
// Shaders/shader programs
//

// Compile a shader from source code. Returns the shader object if the compile succeeds,
// 0 otherwise. Will save the compiler log to compiler_log if set (can be left to NULL).
shader_object_t rend_create_shader(SHADER_TYPE type, const char *source, const char **compiler_log);

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

// Returns the source code for a default shader which renders everything in purple.
const char *rend_get_default_shader_source(void);

//
// Textures
//

// Generate a GPU texture object.
texture_name_t rend_generate_texture(void *image, size_t width, size_t height);

// Destroy a texture on the GPU.
void rend_delete_texture(texture_name_t texture);


#endif
