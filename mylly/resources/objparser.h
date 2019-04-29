#pragma once
#ifndef __OBJPARSER_H
#define __OBJPARSER_H

#include "core/defines.h"
#include "renderer/vertex.h"
#include "collections/array.h"

// -------------------------------------------------------------------------------------------------

// Object vertex defines a position, normal and texture coordinate. These are indices to the
// definitions in the .obj file, where a negative index is from the end of the list.
typedef struct obj_vertex_t {

	int position;
	int normal;
	int texcoord;

} obj_vertex_t;

// -------------------------------------------------------------------------------------------------

typedef struct obj_group_t {

	char *material; // Name of the material used by the submesh
	arr_t(obj_vertex_t) vertices; // Face/vertex definitions with indices to the arrays above.

} obj_group_t;

// -------------------------------------------------------------------------------------------------

typedef struct obj_parser_t {

	char *material_library; // Name of the material library the object uses
	char *material; // Latest material referenced to in the .obj file

	// Arrays of raw vertex data (positions, normals and texture coordinates) defined in the file.
	arr_t(vec3_t) positions;
	arr_t(vec3_t) normals;
	arr_t(vec2_t) texcoords;

	// Object groups (i.e. submeshes)
	arr_t(obj_group_t) groups;

} obj_parser_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// Initialize or destroy an .obj file parser.
void obj_parser_init(obj_parser_t *parser);
void obj_parser_destroy(obj_parser_t *parser);

// Feed a line from an .obj file to the parser.
void obj_parser_process_line(obj_parser_t *parser, const char *line);

// Tell the parser the end of the file has been reached. This will make the parser flush the
// collected data into a model_t structure and return it.
model_t *obj_parser_create_model(obj_parser_t *parser, const char *name, const char *path);

END_DECLARATIONS;

#endif
