#pragma once
#ifndef __RESOURCES_H
#define __RESOURCES_H

#include "core/defines.h"

typedef enum {
	RES_TEXTURE,
	RES_SHADER,
} res_type_t;

typedef struct resource_t {
	uint32_t index;
	uint8_t type; // Type of the resource (res_type_t)
	bool is_loaded; // Indicates whether the resource needs to be loaded separately before use
	char *name;
	char *path;
} resource_t;

typedef struct texture_t texture_t;
typedef struct shader_t shader_t;

// --------------------------------------------------------------------------------

void res_initialize(void);
void res_shutdown(void);

texture_t *res_get_texture(const char *name);
shader_t *res_get_shader(const char *name);

#endif
