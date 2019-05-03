#include "mesh.h"
#include "core/memory.h"
#include "collections/array.h"
#include <assert.h>

// -------------------------------------------------------------------------------------------------

static void mesh_smooth_faces(mesh_t *mesh, bool smooth_normals);

// -------------------------------------------------------------------------------------------------

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

void mesh_calculate_tangents(mesh_t *mesh, bool smooth_shading)
{
	if (mesh == NULL) {
		return;
	}

	// Calculate a tangent for each triangle in the mesh.
	for (uint32_t i = 0; i < mesh->num_indices; i += 3) {

		// Get the indices to the vertex data array for each vertex of the triangle.
		vindex_t i0 = mesh->indices[i + 0];
		vindex_t i1 = mesh->indices[i + 1];
		vindex_t i2 = mesh->indices[i + 2];

		vec3_t vertex0 = mesh->vertices[i0].pos;
		vec3_t vertex1 = mesh->vertices[i1].pos;
		vec3_t vertex2 = mesh->vertices[i2].pos;

		// Calculate triangle normal.
		vec3_t diff1 = vec3_subtract(vertex1, vertex0);
		vec3_t diff2 = vec3_subtract(vertex2, vertex0);

		vec3_t normal = vec3_cross(
			diff1,
			diff2
		);

		vec3_normalize(&normal);

		// Calculate triangle tangent.
		vec3_t delta_pos;

		if (vec3_equals(vertex0, vertex1)) {
			delta_pos = vec3_subtract(vertex2, vertex0);
		}
		else {
			delta_pos = vec3_subtract(vertex1, vertex0);
		}

		vec2_t uv0 = mesh->vertices[i0].uv;
		vec2_t uv1 = mesh->vertices[i1].uv;
		//vec2_t uv2 = mesh->vertices[i2].uv;

		vec2_t delta_uv1 = vec2_subtract(uv1, uv0);
		//vec2_t delta_uv2 = vec2_subtract(uv2, uv0);

		vec3_t tan;

		// Avoid division by zero.
		if (delta_uv1.x != 0) {
			tan = vec3_multiply(delta_pos, 1.0f / delta_uv1.x);
		}
		else {
			tan = delta_pos;
		}

		vec3_normalize(&tan);

		// Assign the calculated normal and tangent to each vertex of the triangle.
		mesh->vertices[i0].tangent = tan;
		mesh->vertices[i1].tangent = tan;
		mesh->vertices[i2].tangent = tan;

		mesh->vertices[i0].normal = normal;
		mesh->vertices[i1].normal = normal;
		mesh->vertices[i2].normal = normal;
	}

	// Smooth the faces by averaging all the tangents (and possibly also normals) of a vertex.
	mesh_smooth_faces(mesh, smooth_shading);

	mesh->is_vertex_data_dirty = true;
}

static void mesh_smooth_faces(mesh_t *mesh, bool smooth_normals)
{
	// A helper structure to store the position of a vertex and all tangents that refer said vertex.
	typedef struct {

		vec3_t vertex; // Position of the vertex

		arr_t(vec3_t) tangents; // List of tangents the vertex has
		arr_t(vec3_t) normals; // List of normals the vertex has

		vec3_t avg_tangent; // Averaged tangent
		vec3_t avg_normal; // Averaged normal

	} vertex_orientation_t;

	// Create an array in which to store all unique vertices and their tangents.
	arr_t(vertex_orientation_t) vertices = arr_initializer;

	// Find all unique vertices and collect a list of normals and tangents the vertex has.
	for (uint32_t i = 0; i < mesh->num_vertices; i++) {

		int index = -1;

		// Try to find a matching vertex in the vertex tangent list.
		for (uint32_t j = 0; j < vertices.count; j++) {

			// TODO: Should the uniqueness of a vertex be determined by the position alone or
			// also its normal?
			if (vec3_equals(vertices.items[j].vertex, mesh->vertices[i].pos)) {

				index = j;
				break;
			}
		}

		// If the vertex was not added to the list yet, create a new entry for it.
		if (index < 0) {
			
			vertex_orientation_t vertex = {
				mesh->vertices[i].pos,
				arr_initializer,
				arr_initializer,
				vec3_zero(),
				vec3_zero()
			};

			arr_push(vertices, vertex);

			index = arr_last_index(vertices);
		}

		// Add tangent and normal to the vertices tangent/normal lists.
		arr_push(vertices.items[index].tangents, mesh->vertices[i].tangent);
		arr_push(vertices.items[index].normals, mesh->vertices[i].normal);
	}

	// Calculate average tangent and normal for each unique vertex.
	for (uint32_t j = 0; j < vertices.count; j++) {

		uint32_t num_tangents = vertices.items[j].tangents.count;
		vec3_t avg_tangent = vec3_zero();
		vec3_t avg_normal = vec3_zero();

		for (uint32_t k = 0; k < num_tangents; k++) { // The number of normals should be the same.

			avg_tangent = vec3_add(avg_tangent, vertices.items[j].tangents.items[k]);

			if (smooth_normals) {
				avg_normal = vec3_add(avg_normal, vertices.items[j].normals.items[k]);
			}
		}

		// Average tangent
		vec3_divide(avg_tangent, num_tangents);
		vec3_normalize(&avg_tangent);

		vertices.items[j].avg_tangent = avg_tangent;

		// Average normals if the mesh uses smooth shading.
		if (smooth_normals) {

			vec3_divide(avg_normal, num_tangents);
			vec3_normalize(&avg_normal);
			vertices.items[j].avg_normal = avg_normal;
		}
	}

	// Assign averaged tangents and normals.
	for (uint32_t i = 0; i < mesh->num_vertices; i++) {

		// Find the averaged tangent/normal for the vertex.
		for (uint32_t j = 0; j < vertices.count; j++) {

			// TODO: Should the uniqueness of a vertex be determined by the position alone or
			// also its normal?
			if (vec3_equals(vertices.items[j].vertex, mesh->vertices[i].pos)) {
				
				mesh->vertices[i].tangent = vertices.items[j].avg_tangent;

				if (smooth_normals) {
					mesh->vertices[i].normal = vertices.items[j].avg_normal;
				}

				break;
			}
		}
	}

	// Clean up.
	for (uint32_t i = 0; i < vertices.count; i++) {

		arr_clear(vertices.items[i].tangents);
		arr_clear(vertices.items[i].normals);
	}

	arr_clear(vertices);
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

void mesh_set_material(mesh_t *mesh, material_t *material)
{
	if (mesh == NULL) {
		return;
	}

	mesh->material = material;

	// Reset override shader and texture when setting a material.
	mesh->shader = NULL;
	mesh->texture = NULL;
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
