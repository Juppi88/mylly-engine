#include "mesh.h"
#include "core/memory.h"
#include <assert.h>

mesh_t *mesh_create(uint32_t index)
{
	NEW(mesh_t, mesh);
	mesh->index = index;

	return mesh;
}

void mesh_destroy(mesh_t *mesh)
{
	assert(mesh != NULL);

	DELETE(mesh->vertices);
	DELETE(mesh->indices);
	DELETE(mesh);
}

void mesh_set_vertices(mesh_t *mesh, const vertex_t *vertices, size_t num_vertices)
{
	assert(mesh != NULL && vertices != NULL);

	NEW_ARRAY(vertex_t, arr, num_vertices);

	for (size_t i = 0; i < num_vertices; ++i) {

		arr[i] = vertices[i];
	}

	mesh->vertices = arr;
	mesh->num_vertices = num_vertices;
}

void mesh_set_indices(mesh_t *mesh, const vindex_t *indices, size_t num_indices)
{
	assert(mesh != NULL && indices != NULL);

	NEW_ARRAY(vindex_t, arr, num_indices);

	for (size_t i = 0; i < num_indices; ++i) {

		arr[i] = indices[i];
	}

	mesh->indices = arr;
	mesh->num_indices = num_indices;
}
