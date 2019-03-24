#pragma once
#ifndef __MODEL_H
#define __MODEL_H

#include "renderer/mesh.h"
#include "collections/array.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef enum {

	PRIMITIVE_QUAD,
	PRIMITIVE_CUBE,

} PRIMITIVE_TYPE;

// -------------------------------------------------------------------------------------------------

typedef struct model_t {

	resource_t resource; // Resource info
	arr_t(mesh_t*) meshes;

} model_t;

// -------------------------------------------------------------------------------------------------

model_t *model_create(const char *name, const char *path);
void model_destroy(model_t *model);

mesh_t *model_add_mesh(model_t *model,
	const vertex_t *vertices, size_t num_vertices,
	const vindex_t *indices, size_t num_indices);

void model_remove_meshes(model_t *model);

void model_setup_primitive(model_t *model, PRIMITIVE_TYPE type);

END_DECLARATIONS;

#endif
