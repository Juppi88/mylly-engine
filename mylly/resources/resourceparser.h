#pragma once
#ifndef __RESOURCEPARSER_H
#define __RESOURCEPARSER_H

#include "core/defines.h"
#include "jsmn/jsmn.h"
#include "math/vector.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

#define MAX_RESOURCE_TOKENS 256

typedef struct res_parser_t {
	jsmn_parser parser;
	jsmntok_t tokens[MAX_RESOURCE_TOKENS];
	int num_tokens;
	const char *text;
} res_parser_t;

// -------------------------------------------------------------------------------------------------

// Initialize a JSON parser helper and parse the raw JSON text into tokens.
// NOTE: The text should stay alive for as long as the parser object.
bool res_parser_init(res_parser_t *parser, const char *text, size_t length);

bool res_parser_is_valid_key_type(res_parser_t *parser, int token);
bool res_parser_is_object(res_parser_t *parser, int token);
bool res_parser_field_equals(res_parser_t *parser, int token, const char *name, uint32_t type);

char *res_parser_get_text(res_parser_t *parser, int token, char *dst, size_t dst_len);
int res_parser_get_int(res_parser_t *parser, int token);
bool res_parser_get_bool(res_parser_t *parser, int token);
float res_parser_get_float(res_parser_t *parser, int token);
vec3_t res_parser_get_vector(res_parser_t *parser, int *token);
colour_t res_parser_get_colour(res_parser_t *parser, int *token);

#endif
