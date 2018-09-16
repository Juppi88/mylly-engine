#pragma once
#ifndef __RENDERVIEW_H
#define __RENDERVIEW_H

#include "core/list.h"
#include "renderer/model.h"
#include "renderer/shader.h"
#include "renderer/vertexbuffer.h"

// --------------------------------------------------------------------------------
// robject_t is a structure which contains data about an object which is visible
// in the current view during this frame.
// --------------------------------------------------------------------------------
typedef struct robject_t {

	LIST_ENTRY(robject_t);
	model_t *model; // The actual render model containing the base meshes
	//mat4 mat;	// Model to world matrix

} robject_t;

// --------------------------------------------------------------------------------
// rmesh_t is a structure which contains the vertices and indices of a single mesh.
// --------------------------------------------------------------------------------
typedef struct rmesh_t {

	LIST_ENTRY(rmesh_t);
	robject_t *parent; // Parent object this mesh belongs to. Contains the model matrix
	vertexbuffer_t *vertices; // Vertex buffer containing the vertices of this mesh
	vertexbuffer_t *indices; // Vertex buffer containing the indices of this mesh
	shader_t *shader; // The shader used for rendering this mesh
} rmesh_t;

// --------------------------------------------------------------------------------
// view_t consists of everything that a single camera renders during the current
// frame. After the frame has been rendered, the rview_t object is invalidated and
// destroyed. This is so in the future we can do view processing and rendering in
// parallel.
// --------------------------------------------------------------------------------
typedef struct rview_t {

	LIST_ENTRY(rview_t);
	LIST(robject_t) objects; // List of visible objects in the view
	LIST(rmesh_t) meshes; // List of all the meshes to be rendered in the view

} rview_t;

// --------------------------------------------------------------------------------

#endif
