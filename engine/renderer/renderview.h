#pragma once
#ifndef __RENDERVIEW_H
#define __RENDERVIEW_H

#include "collections/stack.h"
#include "math/matrix.h"

// --------------------------------------------------------------------------------

typedef struct model_t model_t;
typedef struct vertexbuffer_t vertexbuffer_t;
typedef struct shader_t shader_t;
typedef struct texture_t texture_t;

// --------------------------------------------------------------------------------
// robject_t is a structure which contains data about an object which is visible
// in the current view during this frame.
// --------------------------------------------------------------------------------
typedef struct robject_t {

	stack_entry(robject_t);

	mat_t matrix; // Model to world matrix
	mat_t mvp; // Model-view-projection matrix

} robject_t;

// --------------------------------------------------------------------------------
// rmesh_t is a structure which contains the vertices and indices of a single mesh.
// --------------------------------------------------------------------------------
typedef struct rmesh_t {

	stack_entry(rmesh_t);

	robject_t *parent; // Parent object this mesh belongs to. Contains the model matrices
	vertexbuffer_t *vertices; // Vertex buffer containing the vertices of this mesh
	vertexbuffer_t *indices; // Vertex buffer containing the indices of this mesh
	shader_t *shader; // The shader used for rendering this mesh
	texture_t *texture; // The texture applied to this mesh.

} rmesh_t;

// --------------------------------------------------------------------------------
// view_t consists of everything that a single camera renders during the current
// frame. After the frame has been rendered, the rview_t object is invalidated and
// destroyed. This is so in the future we can do view processing and rendering in
// parallel.
// --------------------------------------------------------------------------------
typedef struct rview_t {

	stack_entry(rview_t);

	mat_t projection; // View-projection matrix

	stack_t(robject_t) objects; // List of visible objects in the view
	stack_t(rmesh_t) meshes; // List of all the meshes to be rendered in the view

} rview_t;

// --------------------------------------------------------------------------------

#endif
