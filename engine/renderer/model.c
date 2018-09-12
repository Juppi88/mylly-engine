#include "model.h"

// --------------------------------------------------------------------------------
// Vertices and indices for common primitives.

static vertex_t quad_vertices[] = {
	{ .pos = vec3(-0.5f, -0.5f, 0.0f), .normal = vec3(0, 0, 1), .uv = vec2(0, 0), .colour = COL_WHITE },
	{ .pos = vec3(0.5f, -0.5f, 0.0f), .normal = vec3(0, 0, 1), .uv = vec2(1, 0), .colour = COL_WHITE },
	{ .pos = vec3(-0.5f, 0.5f, 0.0f), .normal = vec3(0, 0, 1), .uv = vec2(0, 1), .colour = COL_WHITE },
	{ .pos = vec3(0.5f, 0.5f, 0.0f), .normal = vec3(0, 0, 1), .uv = vec2(1, 1), .colour = COL_WHITE }
};

static vindex_t quad_indices[] = {
	0, 1, 2,
	2, 1, 3
};

// --------------------------------------------------------------------------------

model_t *model_create(void)
{
	NEW(model_t, model);
	return model;
}

void model_destroy(model_t *model)
{
	model_remove_meshes(model);
	DELETE(model);
}

mesh_t *model_add_mesh(model_t *model, const vertex_t *vertices, size_t num_vertices, const vindex_t *indices, size_t num_indices)
{
	assert(model != NULL);

	// Create a new mesh and set its vertices.
	mesh_t *mesh = mesh_create(model->meshes.count);

	mesh_set_vertices(mesh, vertices, num_vertices);
	mesh_set_indices(mesh, indices, num_indices);

	// Add the mesh to the model.
	arr_push(model->meshes, mesh);

	return mesh;
}

void model_remove_meshes(model_t *model)
{
	assert(model != NULL);

	mesh_t *mesh;

	arr_foreach(model->meshes, mesh) {

		mesh_destroy(mesh);
	}
	
	arr_clear(model->meshes);
}

void model_setup_primitive(model_t *model, PRIMITIVE_TYPE type)
{
	assert(model != NULL);

	// Remove old meshes.
	model_remove_meshes(model);

	// Add primitive mesh information to the model as a mesh.
	mesh_t *mesh;

	switch (type) {
		case PRIMITIVE_QUAD:
			mesh = mesh_create(0);
			
			mesh_set_vertices(mesh, quad_vertices, LENGTH(quad_vertices));
			mesh_set_indices(mesh, quad_indices, LENGTH(quad_indices));

			arr_push(model->meshes, mesh);
			break;

		default:
			break;
	}
}
