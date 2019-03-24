#pragma once
#ifndef __MTLPARSER_H
#define __MTLPARSER_H

#include "core/defines.h"
#include "collections/array.h"
#include "renderer/material.h"

// -------------------------------------------------------------------------------------------------

typedef struct mtl_parser_t {

	const char *path; // Path to material file
	const char *material; // Name of the material being parsed, NULL if not parsing a material

	arr_t(material_param_t) parameters; // An array of parameters parsed for the current material
	texture_t *diffuse_map; // Diffuse map texture used by the current material
	texture_t *ambient_map; // Diffuse map texture used by the current material

	arr_t(material_t*) materials; // An array of material instances parsed from the file

} mtl_parser_t;

// -------------------------------------------------------------------------------------------------

// Initialize or destroy an .mtl file parser.
void mtl_parser_init(mtl_parser_t *parser, const char *file);
void mtl_parser_destroy(mtl_parser_t *parser);

// Feed a line from an .mtl file to the parser.
void mtl_parser_process_line(mtl_parser_t *parser, const char *line);

// Tell the parser the end of the file has been reached. This will make the parser store the
// material(s) being parsed into the 'materials' array.
void mtl_parser_end_file(mtl_parser_t *parser);

#endif
