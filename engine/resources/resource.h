#pragma once
#ifndef __RESOURCE_H
#define __RESOURCE_H

#include "core/types.h"

// -------------------------------------------------------------------------------------------------

typedef enum {
	RES_TEXTURE,
	RES_SPRITE,
	RES_SHADER,
	RES_ANIMATION,
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

#endif
