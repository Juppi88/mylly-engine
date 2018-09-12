#pragma once
#ifndef __MESH_H
#define __MESH_H

#include "core/defines.h"
#include "renderer/vertex.h"
#include "renderer/vertexbuffer.h"

// --------------------------------------------------------------------------------

typedef struct mesh_t {

	uint32_t index; // Sub-mesh index

	vertex_t *vertices; // Array of vertices in this mesh
	size_t num_vertices;

	vindex_t *indices; // Array of vertex indices
	size_t num_indices;

	vertexbuffer_t *vertex_buffer; // GPU buffer objects
	vertexbuffer_t *index_buffer;

} mesh_t;

// --------------------------------------------------------------------------------

mesh_t *mesh_create(uint32_t index);
void mesh_destroy(mesh_t *mesh);
void mesh_set_vertices(mesh_t *mesh, const vertex_t *vertices, size_t num_vertices);
void mesh_set_indices(mesh_t *mesh, const vindex_t *indices, size_t num_indices);

#endif
