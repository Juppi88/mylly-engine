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
	if (mesh == NULL) {
		return;
	}

	DELETE(mesh->vertices);
	DELETE(mesh->indices);
	DELETE(mesh);
}

void mesh_set_vertices(mesh_t *mesh, const vertex_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data.
	if (mesh->vertices != NULL) {

		DELETE(mesh->vertices);

		mesh->vertices = NULL;
		mesh->num_vertices = 0;
		mesh->vertex_buffer = NULL;
	}

	NEW_ARRAY(vertex_t, arr, num_vertices);

	for (size_t i = 0; i < num_vertices; ++i) {

		arr[i] = vertices[i];
	}

	mesh->vertices = arr;
	mesh->num_vertices = num_vertices;
	mesh->is_vertex_data_dirty = true;
	mesh->vertex_type = VERTEX_NORMAL;
	mesh->vertex_size = sizeof(vertex_t);
}

void mesh_set_particle_vertices(mesh_t *mesh, const vertex_particle_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data.
	if (mesh->part_vertices != NULL) {

		DELETE(mesh->part_vertices);

		mesh->part_vertices = NULL;
		mesh->num_vertices = 0;
		mesh->vertex_buffer = NULL;
	}

	NEW_ARRAY(vertex_particle_t, arr, num_vertices);

	for (size_t i = 0; i < num_vertices; ++i) {

		arr[i] = vertices[i];
	}

	mesh->part_vertices = arr;
	mesh->num_vertices = num_vertices;
	mesh->is_vertex_data_dirty = true;
	mesh->vertex_type = VERTEX_PARTICLE;
	mesh->vertex_size = sizeof(vertex_particle_t);
}

void mesh_refresh_vertices(mesh_t *mesh)
{
	if (mesh == NULL || mesh->vertices == NULL) {
		return;
	}

	mesh->is_vertex_data_dirty = true;
}

void mesh_set_indices(mesh_t *mesh, const vindex_t *indices, size_t num_indices)
{
	if (mesh == NULL || indices == NULL) {
		return;
	}

	// Remove old index data.
	if (mesh->indices != NULL) {

		DELETE(mesh->indices);

		mesh->indices = NULL;
		mesh->num_indices = 0;
		mesh->index_buffer = NULL;
	}

	NEW_ARRAY(vindex_t, arr, num_indices);

	for (size_t i = 0; i < num_indices; ++i) {

		arr[i] = indices[i];
	}

	mesh->indices = arr;
	mesh->num_indices = num_indices;
}

void mesh_set_material(mesh_t *mesh, shader_t *shader, texture_t *texture)
{
	if (mesh == NULL) {
		return;
	}

	mesh->texture = texture;
	mesh->shader = shader;
}
