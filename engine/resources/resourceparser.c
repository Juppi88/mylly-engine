#include "resourceparser.h"
#include "core/string.h"
#include "math/math.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

bool res_parser_init(res_parser_t *parser, const char *text, size_t length)
{
	if (parser == NULL || text == NULL) {
		return false;
	}

	// Initialize a JSON parser object.
	jsmn_init(&parser->parser);

	// Parse the input text.
	parser->text = text;
	parser->num_tokens =
		jsmn_parse(&parser->parser, text, length, parser->tokens, MAX_RESOURCE_TOKENS);

	// Negative token count indicates an error -> not a valid JSON formatted resource file.
	if (parser->num_tokens < 0) {
		return false;
	}

	// Assume the top-level element is always an object.
	if (parser->num_tokens == 0 || parser->tokens[0].type != JSMN_OBJECT) {
		return false;
	}

	return true;
}

bool res_parser_is_valid_key_type(res_parser_t *parser, int token)
{
	if (parser == NULL || token >= parser->num_tokens) {
		return false;
	}

	return (parser->tokens[token].type == JSMN_STRING ||
		    parser->tokens[token].type == JSMN_PRIMITIVE);
}

bool res_parser_is_object(res_parser_t *parser, int token)
{
	if (parser == NULL || token + 1 >= parser->num_tokens) {
		return false;
	}

	return (parser->tokens[token].type == JSMN_OBJECT);
}

bool res_parser_field_equals(res_parser_t *parser, int token, const char *name, int type)
{
	if (parser == NULL || name == NULL || token + 1 >= parser->num_tokens) {
		return false;
	}

	const char *key = &parser->text[parser->tokens[token].start];
	size_t name_len = strlen(name);

	return (string_equals_len(name, key, name_len) &&
            parser->tokens[token + 1].type == type);
}

char *res_parser_get_text(res_parser_t *parser, int token, char *dst, size_t dst_len)
{
	if (parser == NULL || dst == NULL || dst_len == 0 || token >= parser->num_tokens) {

		if (dst && dst_len) {
			*dst = 0;
		}

		return dst;
	}

	dst_len = MIN(dst_len, parser->tokens[token].end - parser->tokens[token].start + 1);
	string_copy(dst, &parser->text[parser->tokens[token].start], dst_len);

	return dst;
}

int res_parser_get_int(res_parser_t *parser, int token)
{
	if (parser == NULL || token >= parser->num_tokens) {
		return 0;
	}

	char tmp[100];
	size_t length = MIN(sizeof(tmp), parser->tokens[token].end - parser->tokens[token].start + 1);

	string_copy(tmp, &parser->text[parser->tokens[token].start], length);

	return atoi(tmp);
}

bool res_parser_get_bool(res_parser_t *parser, int token)
{
	if (parser == NULL || token >= parser->num_tokens) {
		return false;
	}

	char tmp[100];
	size_t length = MIN(sizeof(tmp), parser->tokens[token].end - parser->tokens[token].start + 1);

	string_copy(tmp, &parser->text[parser->tokens[token].start], length);

	return string_equals(tmp, "true");
}

float res_parser_get_float(res_parser_t *parser, int token)
{
	if (parser == NULL || token >= parser->num_tokens) {
		return 0.0f;
	}

	char tmp[100];
	size_t length = MIN(sizeof(tmp), parser->tokens[token].end - parser->tokens[token].start + 1);

	string_copy(tmp, &parser->text[parser->tokens[token].start], length);

	return atof(tmp);
}
