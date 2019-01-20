#pragma once
#ifndef __MESH_H
#define __MESH_H

#include "core/defines.h"
#include "renderer/vertex.h"
#include "renderer/vertexbuffer.h"
#include "renderer/buffercache.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct shader_t shader_t;
typedef struct texture_t texture_t;

// -------------------------------------------------------------------------------------------------

typedef struct mesh_t {

	uint32_t index; // Sub-mesh index

	vertex_type_t vertex_type; // The type of vertex used by the mesh
	size_t vertex_size; // The size of one vertex

	// Array of vertices in this mesh. Array depends on the type of vertex used.
	union {
		vertex_t *vertices;
		vertex_particle_t *part_vertices;
		vertex_ui_t *ui_vertices;
	};

	size_t num_vertices;
	bool is_vertex_data_dirty; // Set to true when the data on the GPU needs refreshing

	vindex_t *indices; // Array of vertex indices
	size_t num_indices;

	// GPU buffer objects.
	// TODO: Replace with buffer handles eventually.
	vertexbuffer_t *vertex_buffer;
	vertexbuffer_t *index_buffer;

	// Handles to GPU copy of vertices.
	buffer_handle_t handle_vertices;
	buffer_handle_t handle_indices;

	// TODO: Move these to a customizable material structure.
	shader_t *shader; // Shader used for rendering this mesh
	texture_t *texture; // Texture on this mesh

} mesh_t;

// -------------------------------------------------------------------------------------------------

mesh_t *mesh_create(void);
void mesh_destroy(mesh_t *mesh);

// Use one of these methods to set the vertices of the mesh, depending on the type of the mesh.
void mesh_set_vertices(mesh_t *mesh, const vertex_t *vertices, size_t num_vertices);
void mesh_set_particle_vertices(mesh_t *mesh, const vertex_particle_t *vertices, size_t num_vertices);
void mesh_set_ui_vertices(mesh_t *mesh, const vertex_ui_t *vertices, size_t num_vertices);

void mesh_prealloc_vertices(mesh_t *mesh, vertex_type_t type, size_t num_vertices);
void mesh_refresh_vertices(mesh_t *mesh);

void mesh_set_indices(mesh_t *mesh, const vindex_t *indices, size_t num_indices);

void mesh_set_material(mesh_t *mesh, shader_t *shader, texture_t *texture);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
