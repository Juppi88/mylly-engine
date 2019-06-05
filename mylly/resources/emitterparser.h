#pragma once
#ifndef __EMITTERPARSER_H
#define __EMITTERPARSER_H

#include "core/defines.h"
#include "collections/array.h"
#include "resources/resourceparser.h"

// -------------------------------------------------------------------------------------------------

typedef struct emitter_parser_t {

	const char *path; // Path to emitter file
	res_parser_t parser; // JSON resource parser

	arr_t(emitter_t*) emitters; // A list of parsed emitters

} emitter_parser_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// Initialize or destroy a parser for a particle emitter.
bool emitter_parser_init(emitter_parser_t *parser, const char *path,
                         const char *contents, size_t contents_length);

void emitter_parser_destroy(emitter_parser_t *parser);

// Tell the parser the end of the file has been reached. This will make the parser store the
// emitters being parsed into the 'emitters' array.
void emitter_parser_end_file(emitter_parser_t *parser);

END_DECLARATIONS;

#endif
