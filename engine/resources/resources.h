#pragma once
#ifndef __RESOURCES_H
#define __RESOURCES_H

#include "core/defines.h"
#include "core/types.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef enum {
	RES_TEXTURE,
	RES_SPRITE,
	RES_SHADER,
} res_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct resource_t {
	uint32_t index;
	uint8_t type; // Type of the resource (res_type_t)
	bool is_loaded; // Indicates whether the resource needs to be loaded separately before use
	char *name;
	char *path;
} resource_t;

// -------------------------------------------------------------------------------------------------

void res_initialize(void);
void res_shutdown(void);

texture_t *res_get_texture(const char *name);
sprite_t *res_get_sprite(const char *name);
shader_t *res_get_shader(const char *name);

END_DECLARATIONS;

#endif
