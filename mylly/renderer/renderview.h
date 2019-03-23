#pragma once
#ifndef __RENDERVIEW_H
#define __RENDERVIEW_H

#include "collections/list.h"
#include "math/matrix.h"
#include "renderer/shader.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

typedef struct model_t model_t;
typedef struct vertexbuffer_t vertexbuffer_t;
typedef struct shader_t shader_t;
typedef struct texture_t texture_t;

// -------------------------------------------------------------------------------------------------
// robject_t is a structure which contains data about an object which is visible
// in the current view during this frame.
// -------------------------------------------------------------------------------------------------
typedef struct robject_t {

	list_entry(robject_t);

	mat_t matrix; // Model to world matrix
	mat_t mvp; // Model-view-projection matrix

} robject_t;

// -------------------------------------------------------------------------------------------------
// rmesh_t is a structure which contains the vertices and indices of a single mesh.
// -------------------------------------------------------------------------------------------------
typedef struct rmesh_t {

	list_entry(rmesh_t);

	robject_t *parent; // Parent object this mesh belongs to. Contains the model matrices
	vertex_type_t vertex_type; // The type of vertex used by the mesh

	// Vertex and index data. Only one of the two is used, either buffer or handle to buffer.
	vertexbuffer_t *vertices; // Vertex buffer containing the vertices of this mesh
	vertexbuffer_t *indices; // Vertex buffer containing the indices of this mesh
	buffer_handle_t handle_vertices; // Handle to vertex data
	buffer_handle_t handle_indices; // Handle to index data
	
	shader_t *shader; // The shader used for rendering this mesh
	texture_t *texture; // The texture applied to this mesh.

} rmesh_t;

// -------------------------------------------------------------------------------------------------
// view_t consists of everything that a single camera renders during the current
// frame. After the frame has been rendered, the rview_t object is invalidated and
// destroyed. This is so in the future we can do view processing and rendering in
// parallel.
// -------------------------------------------------------------------------------------------------
typedef struct rview_t {

	list_entry(rview_t);

	mat_t projection; // View-projection matrix

	// List of visible objects in the view
	list_t(robject_t) objects;
	
	// List of all the meshes to be rendered in the view, sorted by render queues
	list_t(rmesh_t) meshes[NUM_QUEUES];

	// A virtual root object placed at the origin. Used for custom meshes placed in the scene
	// (for example debug meshes).
	robject_t root;

} rview_t;

// -------------------------------------------------------------------------------------------------

#endif
