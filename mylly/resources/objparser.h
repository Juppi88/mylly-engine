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

typedef struct obj_parser_t {

	// Arrays of raw vertex data (positions, normals and texture coordinates).
	arr_t(vec3_t) positions;
	arr_t(vec3_t) normals;
	arr_t(vec2_t) texcoords;

	// Face/vertex definitions with indices to the arrays above.
	arr_t(obj_vertex_t) vertices;

	char *object_name; // Name of the object being parsed
	char *material_library; // Name of the material library the object uses
	char *material; // Name of the material used by the current submesh

} obj_parser_t;

// -------------------------------------------------------------------------------------------------

// Initialize or destroy an .obj file parser.
void obj_parser_init(obj_parser_t *parser);
void obj_parser_destroy(obj_parser_t *parser);

// Feed a line from an .obj file to the parser.
void obj_parser_process_line(obj_parser_t *parser, const char *line);

// Creates a model from the parsed object data.
model_t *obj_parser_create_model(obj_parser_t *parser, const char *name, const char *path);

#endif
