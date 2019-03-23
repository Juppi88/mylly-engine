#include "mesh.h"
#include "core/memory.h"
#include <assert.h>

mesh_t *mesh_create(void)
{
	NEW(mesh_t, mesh);
	return mesh;
}

void mesh_destroy(mesh_t *mesh)
{
	if (mesh == NULL) {
		return;
	}

	DESTROY(mesh->vertices);
	DESTROY(mesh->indices);
	DESTROY(mesh);
}

void mesh_set_vertices(mesh_t *mesh, const vertex_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data if vertex type has changed or the size of the buffer is different.
	if (mesh->vertices == NULL ||
		mesh->vertex_type != VERTEX_NORMAL ||
		mesh->num_vertices != num_vertices) {

		if (mesh->vertices != NULL) {

			DESTROY(mesh->vertices);

			mesh->vertices = NULL;
			mesh->num_vertices = 0;
			mesh->vertex_buffer = NULL;
		}

		NEW_ARRAY(vertex_t, arr, num_vertices);

		mesh->vertices = arr;
		mesh->num_vertices = num_vertices;
		mesh->vertex_type = VERTEX_NORMAL;
		mesh->vertex_size = sizeof(vertex_t);
	}

	// Copy new vertices into the buffer.
	for (size_t i = 0; i < num_vertices; ++i) {
		mesh->vertices[i] = vertices[i];
	}
	
	mesh->is_vertex_data_dirty = true;
}

void mesh_set_particle_vertices(mesh_t *mesh, const vertex_particle_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data if vertex type has changed or the size of the buffer is different.
	if (mesh->part_vertices == NULL ||
		mesh->vertex_type != VERTEX_PARTICLE ||
		mesh->num_vertices != num_vertices) {

		if (mesh->part_vertices != NULL) {

			DESTROY(mesh->part_vertices);

			mesh->part_vertices = NULL;
			mesh->num_vertices = 0;
			mesh->vertex_buffer = NULL;
		}

		NEW_ARRAY(vertex_particle_t, arr, num_vertices);

		mesh->part_vertices = arr;
		mesh->num_vertices = num_vertices;
		mesh->vertex_type = VERTEX_PARTICLE;
		mesh->vertex_size = sizeof(vertex_particle_t);
	}

	// Copy new vertices into the buffer.
	for (size_t i = 0; i < num_vertices; ++i) {
		mesh->part_vertices[i] = vertices[i];
	}

	mesh->is_vertex_data_dirty = true;
}

void mesh_set_ui_vertices(mesh_t *mesh, const vertex_ui_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data if vertex type has changed or the size of the buffer is different.
	if (mesh->ui_vertices == NULL ||
		mesh->vertex_type != VERTEX_UI ||
		mesh->num_vertices != num_vertices) {

		if (mesh->ui_vertices != NULL) {

			DESTROY(mesh->ui_vertices);

			mesh->ui_vertices = NULL;
			mesh->num_vertices = 0;
			mesh->vertex_buffer = NULL;
		}

		NEW_ARRAY(vertex_ui_t, arr, num_vertices);

		mesh->ui_vertices = arr;
		mesh->num_vertices = num_vertices;
		mesh->vertex_type = VERTEX_UI;
		mesh->vertex_size = sizeof(vertex_ui_t);
	}

	// Copy new vertices into the buffer.
	for (size_t i = 0; i < num_vertices; ++i) {
		mesh->ui_vertices[i] = vertices[i];
	}
	
	mesh->is_vertex_data_dirty = true;
}

void mesh_set_debug_vertices(mesh_t *mesh, const vertex_debug_t *vertices, size_t num_vertices)
{
	if (mesh == NULL || vertices == NULL) {
		return;
	}

	// Remove old vertex data if vertex type has changed or the size of the buffer is different.
	if (mesh->ui_vertices == NULL ||
		mesh->vertex_type != VERTEX_DEBUG ||
		mesh->num_vertices != num_vertices) {

		if (mesh->ui_vertices != NULL) {

			DESTROY(mesh->ui_vertices);

			mesh->ui_vertices = NULL;
			mesh->num_vertices = 0;
			mesh->vertex_buffer = NULL;
		}

		NEW_ARRAY(vertex_debug_t, arr, num_vertices);

		mesh->debug_vertices = arr;
		mesh->num_vertices = num_vertices;
		mesh->vertex_type = VERTEX_DEBUG;
		mesh->vertex_size = sizeof(vertex_debug_t);
	}

	// Copy new vertices into the buffer.
	for (size_t i = 0; i < num_vertices; ++i) {
		mesh->debug_vertices[i] = vertices[i];
	}
	
	mesh->is_vertex_data_dirty = true;
}

void mesh_prealloc_vertices(mesh_t *mesh, vertex_type_t type, size_t num_vertices)
{
	if (mesh == NULL) {
		return;
	}

	// Destroy existing buffer to avoid leaks.
	if (mesh->vertices != NULL) {
		DESTROY(mesh->vertices);
	}

	if (type == VERTEX_NORMAL) {

		NEW_ARRAY(vertex_t, arr, num_vertices);
		
		mesh->vertices = arr;
		mesh->vertex_size = sizeof(vertex_t);
	}
	else if (type == VERTEX_PARTICLE) {

		NEW_ARRAY(vertex_particle_t, arr, num_vertices);

		mesh->part_vertices = arr;
		mesh->vertex_size = sizeof(vertex_particle_t);
	}
	else if (type == VERTEX_UI) {

		NEW_ARRAY(vertex_ui_t, arr, num_vertices);

		mesh->ui_vertices = arr;
		mesh->vertex_size = sizeof(vertex_ui_t);

		// Allocate the GPU handle for UI vertices.
		mesh->handle_vertices = bufcache_alloc_vertices(BUFIDX_UI, mesh->ui_vertices,
		                                                mesh->vertex_size, num_vertices);
	}
	else if (type == VERTEX_DEBUG) {

		NEW_ARRAY(vertex_debug_t, arr, num_vertices);

		mesh->debug_vertices = arr;
		mesh->vertex_size = sizeof(vertex_debug_t);

		// Allocate the GPU handle for UI vertices.
		mesh->handle_vertices = bufcache_alloc_vertices(BUFIDX_DEBUG_LINE, mesh->debug_vertices,
		                                                mesh->vertex_size, num_vertices);
	}

	mesh->vertex_type = type;
	mesh->num_vertices = num_vertices;
	mesh->is_vertex_data_dirty = true;
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

		DESTROY(mesh->indices);

		mesh->indices = NULL;
		mesh->num_indices = 0;
		mesh->index_buffer = NULL;
	}

	// If the vertices are stored into a shared buffer object, take the start index into account.
	vindex_t index_offset = 0;

	if (mesh->handle_vertices != 0) {
		index_offset = (vindex_t)BUFFER_GET_START_INDEX(mesh->handle_vertices, mesh->vertex_size);
	}

	NEW_ARRAY(vindex_t, arr, num_indices);

	for (size_t i = 0; i < num_indices; ++i) {
		arr[i] = indices[i] + index_offset;
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

void mesh_set_shader(mesh_t *mesh, shader_t *shader)
{
	if (mesh == NULL) {
		return;
	}

	mesh->shader = shader;
}

void mesh_set_texture(mesh_t *mesh, texture_t *texture)
{
	if (mesh == NULL) {
		return;
	}

	mesh->texture = texture;
}
