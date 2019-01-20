#include "model.h"

// -------------------------------------------------------------------------------------------------
//
// Vertices and indices for common primitives.
//
// All the vector/colour construction macros are unwrapped because Visual Studio does not seem to
// understand that the data passed to the macros is constant, causing a C2099 'initializer is not
// a constant' error.
//

static vertex_t quad_vertices[] = {

	{ .pos = { .vec = { -0.5f, -0.5f, 0.0f, 1.0f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, -0.5f, 0.0f, 1.0f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { -0.5f, 0.5f, 0.0f, 1.0f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 1 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, 0.5f, 0.0f, 1.0f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 1 } }, .colour = { 255, 255, 255, 255 } }
	
};

static vindex_t quad_indices[] = {
	0, 1, 2,
	2, 1, 3
};

static vertex_t cube_vertices[] = {

	// Side vertices
	{ .pos = { .vec = { -0.5f, -0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, -0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { -0.5f, 0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 1 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, 0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 1 } }, .colour = { 255, 255, 255, 255 } },

	{ .pos = { .vec = { -0.5f, -0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, -0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { -0.5f, 0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 1 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, 0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 1 } }, .colour = { 255, 255, 255, 255 } },

	// Bottom vertices
	{ .pos = { .vec = { -0.5f, -0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, -0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { -0.5f, -0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 1 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, -0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 1 } }, .colour = { 255, 255, 255, 255 } },

	// Top vertices
	{ .pos = { .vec = { -0.5f, 0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, 0.5f, -0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 0 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { -0.5f, 0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 0, 1 } }, .colour = { 255, 255, 255, 255 } },
	{ .pos = { .vec = { 0.5f, 0.5f, 0.5f } }, .normal = { .vec = { 0, 0, 1 } }, .uv = { .vec = { 1, 1 } }, .colour = { 255, 255, 255, 255 } },

};

static vindex_t cube_indices[] = {
	0, 1, 2, 2, 1, 3,
	5, 4, 7, 7, 4, 6,
	1, 5, 3, 3, 5, 7,
	4, 0, 6, 6, 0, 2,
	12, 13, 14, 14, 13, 15,
	8, 9, 10, 10, 9, 11
};

// -------------------------------------------------------------------------------------------------

model_t *model_create(void)
{
	NEW(model_t, model);
	return model;
}

void model_destroy(model_t *model)
{
	if (model == NULL) {
		return;
	}

	// Remove all sub-meshes first.
	model_remove_meshes(model);

	DESTROY(model);
}

mesh_t *model_add_mesh(
	model_t *model,
	const vertex_t *vertices, size_t num_vertices,
	const vindex_t *indices, size_t num_indices)
{
	if (model == NULL) {
		return NULL;
	}

	// Create a new mesh and set its vertices.
	mesh_t *mesh = mesh_create();

	mesh_set_vertices(mesh, vertices, num_vertices);
	mesh_set_indices(mesh, indices, num_indices);

	// Add the mesh to the model.
	arr_push(model->meshes, mesh);

	return mesh;
}

void model_remove_meshes(model_t *model)
{
	if (model == NULL) {
		return;
	}

	mesh_t *mesh;

	arr_foreach(model->meshes, mesh) {

		mesh_destroy(mesh);
	}
	
	arr_clear(model->meshes);
}

void model_set_material(model_t *model, int mesh, shader_t *shader, texture_t *texture)
{
	if (model == NULL) {
		return;
	}


	if (mesh < 0) {

		// Set the material for every submesh of the model.
		mesh_t *submesh;

		arr_foreach(model->meshes, submesh) {
			mesh_set_material(submesh, shader, texture);
		}
	}
	else if (mesh >= 0 && (size_t)mesh < model->meshes.count) {

		// Set the material of a specific submesh.
		mesh_t *submesh = model->meshes.items[mesh];

		if (submesh != NULL) {
			mesh_set_material(submesh, shader, texture);
		}
	}
}

void model_setup_primitive(model_t *model, PRIMITIVE_TYPE type)
{
	if (model == NULL) {
		return;
	}

	// Remove old meshes.
	model_remove_meshes(model);

	// Add primitive mesh information to the model as a mesh.
	mesh_t *mesh;

	switch (type) {
		case PRIMITIVE_QUAD:
			mesh = mesh_create();
			
			mesh_set_vertices(mesh, quad_vertices, LENGTH(quad_vertices));
			mesh_set_indices(mesh, quad_indices, LENGTH(quad_indices));

			arr_push(model->meshes, mesh);
			break;

		case PRIMITIVE_CUBE:
			mesh = mesh_create();
			
			mesh_set_vertices(mesh, cube_vertices, LENGTH(cube_vertices));
			mesh_set_indices(mesh, cube_indices, LENGTH(cube_indices));

			arr_push(model->meshes, mesh);
			break;

		default:
			break;
	}
}
