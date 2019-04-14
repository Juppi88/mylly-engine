#include "mtlparser.h"
#include "io/log.h"
#include "core/string.h"
#include "resources/resources.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static void mtl_parser_add_parameter(mtl_parser_t *parser, size_t num_values,
                                     const char *name, shader_data_type_t type);

static const shader_uniform_t *mtl_parser_get_uniform(const char *name, shader_data_type_t type);
static void mtl_parser_save_material(mtl_parser_t *parser);

// -------------------------------------------------------------------------------------------------

void mtl_parser_init(mtl_parser_t *parser, const char *file)
{
	if (parser == NULL) {
		return;
	}

	arr_init(parser->materials);
	arr_init(parser->parameters);

	// Store the path of the material file and the name of the file for creating material instances.
	parser->path = string_duplicate(file);
	parser->material = NULL;

	parser->diffuse_map = NULL;
	parser->normal_map = NULL;
}

void mtl_parser_destroy(mtl_parser_t *parser)
{
	if (parser == NULL) {
		return;
	}

	arr_clear(parser->materials);
	arr_clear(parser->parameters);

	DESTROY(parser->path);
	DESTROY(parser->material);
}

void mtl_parser_process_line(mtl_parser_t *parser, const char *line)
{
	if (parser == NULL) {
		return;
	}

	// Ignore empty lines and comments.
	if (string_is_null_or_empty(line) ||
		*line == '#') {

		return;
	}

	// Count the number of tokens in the line.
	size_t num_tokens = string_token_count(line, ' ');

	// Get the type of data represented by the line.
	char type[32];
	char text[260];
	char texture_name[260];

	string_tokenize(line, ' ', type, sizeof(type));

	// Parse the line based on the type.
	if (string_equals(type, "newmtl")) { // newmtl = Start new material instance

		// If we've been already been parsing a material, dump it into a file.
		mtl_parser_save_material(parser);

		// Save the name of the new material.
		string_tokenize(NULL, ' ', text, sizeof(text));
		parser->material = string_duplicate(text);
	}
	else if (string_equals(type, "Ns")) { // Ns = specular weight (1 float, ranged between 0...1000)
		mtl_parser_add_parameter(parser, num_tokens - 1, "SpecularWeight", DATATYPE_FLOAT);
	}
	else if (string_equals(type, "Ka")) { // Ka = ambient colour (3 floats)
		mtl_parser_add_parameter(parser, num_tokens - 1, "AmbientColour", DATATYPE_COLOUR);
	}
	else if (string_equals(type, "Kd")) { // Kd = diffuse colour (3 floats)
		mtl_parser_add_parameter(parser, num_tokens - 1, "DiffuseColour", DATATYPE_COLOUR);
	}
	else if (string_equals(type, "Ks")) { // Ks = specular colour (3 floats)
		mtl_parser_add_parameter(parser, num_tokens - 1, "SpecularColour", DATATYPE_COLOUR);
	}
	else if (string_equals(type, "Ke")) { // Ke = emissive (PBR) (3 floats)
		mtl_parser_add_parameter(parser, num_tokens - 1, "EmissiveColour", DATATYPE_COLOUR);
	}
	else if (string_equals(type, "Ni")) { // Ni = optical density (1 float, 0.001...10)
		mtl_parser_add_parameter(parser, num_tokens - 1, "OpticalDensity", DATATYPE_FLOAT);
	}
	else if (string_equals(type, "d")) { // d = dissolved (transparency) (1 float 0...1, 1 = opaque)
		mtl_parser_add_parameter(parser, num_tokens - 1, "Dissolved", DATATYPE_FLOAT);
	}
	else if (string_equals(type, "illum")) { // illum = illumination model, enumeration
		// Illumination model is currently not supported by the parser.
	}
	else if (string_equals(type, "map_Kd")) { // map_Kd = diffuse map texture

		// Get the name of the texture without extension.
		string_tokenize(NULL, ' ', text, sizeof(text));
		string_get_file_name_without_extension(text, texture_name, sizeof(texture_name));

		parser->diffuse_map = res_get_texture(texture_name);
	}
	else if (string_equals(type, "norm")) { // norm = normal map texture

		string_tokenize(NULL, ' ', text, sizeof(text));
		string_get_file_name_without_extension(text, texture_name, sizeof(texture_name));

		parser->normal_map = res_get_texture(texture_name);
	}
	else {
		// The line contains some type of data not handled by this parser, warn the user about it.
		log_warning(".mtl parser", "Unhandled data line: %s.", type);
	}
}

static const shader_uniform_t *mtl_parser_get_uniform(const char *name, shader_data_type_t type)
{
	uint32_t index;

	// Create a new uniform definition if one by the given name doesn't exist.
	if (!shader_data_uniform_exists(name, &index)) {
		index = shader_data_add_uniform(name, type);
	}

	return shader_data_get_uniform_by_index(index);
}

void mtl_parser_end_file(mtl_parser_t *parser)
{
	if (parser == NULL) {
		return;
	}

	// Save the material being parsed.
	mtl_parser_save_material(parser);
}

static void mtl_parser_add_parameter(mtl_parser_t *parser, size_t num_values,
                                     const char *name, shader_data_type_t type)
{
	const shader_uniform_t *uniform = mtl_parser_get_uniform(name, type);

	if (uniform == NULL) {
		return;
	}

	// Ensure the line has enough values to parse from.
	switch (uniform->type) {
		case DATATYPE_INT:
		case DATATYPE_FLOAT:
			if (num_values < 1) {
				return;
			}
			break;

		case DATATYPE_VECTOR4:
			if (num_values < 4) {
				return;
			}
			break;

		case DATATYPE_COLOUR:
			if (num_values < 3) {
				return;
			}
			break;

		default:
			log_warning(".mtl parser", "Unhandled data type in material file (%u).", uniform->type);
			return;
	}

	// Parse the rest of the line based on the type of data.
	material_value_t value;
	char str1[32];
	char str2[32];
	char str3[32];
	char str4[32];
	float r, g, b, a;

	switch (uniform->type) {
		case DATATYPE_INT:
			string_tokenize(NULL, ' ', str1, sizeof(str1));

			value.int_value = atoi(str1);
			break;

		case DATATYPE_FLOAT:
			string_tokenize(NULL, ' ', str1, sizeof(str1));

			value.float_value = atof(str1);
			break;

		case DATATYPE_VECTOR4:
			string_tokenize(NULL, ' ', str1, sizeof(str1));
			string_tokenize(NULL, ' ', str2, sizeof(str2));
			string_tokenize(NULL, ' ', str3, sizeof(str3));
			string_tokenize(NULL, ' ', str4, sizeof(str4));

			value.vec4_value = vec4(atof(str1), atof(str2), atof(str3), atof(str4));
			break;

		case DATATYPE_COLOUR:
			string_tokenize(NULL, ' ', str1, sizeof(str1));
			string_tokenize(NULL, ' ', str2, sizeof(str2));
			string_tokenize(NULL, ' ', str3, sizeof(str3));
			string_tokenize(NULL, ' ', str4, sizeof(str4));

			r = atof(str1);
			g = atof(str2);
			b = atof(str3);
			a = (string_is_null_or_empty(str4) ? 1.0f : atof(str4)); // Use 1 as default alpha

			// The colours in .mtl files are normalized. Convert these to 0...255 values.
			value.colour_value = col_a(
				(uint8_t)(255 * r),
				(uint8_t)(255 * g),
				(uint8_t)(255 * b),
				(uint8_t)(255 * a)
			);
			break;

		default:
			return;
	}

	// Add the parameter to the list of parsed parameters.
	material_param_t param = {
		uniform,
		value
	};

	arr_push(parser->parameters, param);
}

static void mtl_parser_save_material(mtl_parser_t *parser)
{
	// Dump parsed data into a material_t instance and store it to the materials array
	// for later accessing.
	if (!string_is_null_or_empty(parser->material)) {

		material_t *material = material_create(parser->material, parser->path);

		// Copy parameters into the new material instance.
		for (size_t i = 0; i < parser->parameters.count; i++) {
			arr_push(material->parameters, parser->parameters.items[i]);
		}

		// Copy texture references.
		material->texture = parser->diffuse_map;
		material->normal_map = parser->normal_map;

		// Add to an array of materials parsed from this .mtl file.
		arr_push(parser->materials, material);
	}

	// Reset parser storages for a new material.
	arr_clear(parser->parameters);
	DESTROY(parser->material);

	parser->diffuse_map = NULL;
	parser->normal_map = NULL;
}
