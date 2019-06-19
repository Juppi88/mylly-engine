#pragma once
#ifndef __RESOURCE_H
#define __RESOURCE_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef enum {

	RES_TEXTURE,
	RES_SPRITE,
	RES_SHADER,
	RES_ANIMATION,
	RES_FONT,
	RES_MODEL,
	RES_MATERIAL,
	RES_EMITTER,
	RES_SOUND,

} res_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct resource_t {

	uint32_t index;
	uint8_t type; // Type of the resource (res_type_t)
	bool is_loaded; // Indicates whether the resource needs to be loaded separately before use

	char *res_name; // The full name of the resource, prefixed by the name of the parent
	char *name; // Name of the resource, without parent name
	char *path; // File path to resource. NULL for resources which are part of a parent resource

} resource_t;

// -------------------------------------------------------------------------------------------------

#endif
