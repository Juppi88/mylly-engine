#pragma once
#ifndef __VB_CACHE_H
#define __VB_CACHE_H

#include "renderer/vertexbuffer.h"

// The number of new buffers to allocate when allovating new VBOs.
#define VBCACHE_ADDITIONAL_BUFFERS (1 << 10)

// The number of frames a buffer exists if not used.
#define VBCACHE_LIFE_TIME 2

// --------------------------------------------------------------------------------

void vbcache_initialize(void);
void vbcache_shutdown(void);

void vbcache_set_current_frame(int frame);

void vbcache_alloc_buffer(void *data, size_t num_elements, size_t elem_size, vertexbuffer_t **storage,
						  bool is_index_data);

void vbcache_refresh_buffer(vertexbuffer_t *buffer);

void vbcache_free_inactive_buffers(void);

#endif
