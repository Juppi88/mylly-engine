#pragma once
#ifndef __RENDERER_H
#define __RENDERER_H

#include "core/defines.h"
#include "renderer/model.h"
#include "renderer/renderview.h"
#include "renderer/vertexbuffer.h"

bool rend_initialize(void);
void rend_shutdown(void);

void rend_draw_views(LIST(rview_t) views);

// Generate a new vertex buffer object. Will be freed automatically after a while unless
// explicitly refreshed.
vbindex_t rend_generate_buffer(void);

// Destroy a generated vertex buffer object. Should only be called on renderer shutdown.
void rend_destroy_buffer(vbindex_t vbo);

// Upload vertex/index data to the GPU. Size of the buffer is the number of elements.
void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index_data);

#endif
