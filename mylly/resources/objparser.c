#include "objparser.h"
#include "io/log.h"
#include "core/string.h"
#include "renderer/mesh.h"
#include "scene/model.h"
#include "resources/resources.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static void obj_parser_process_face(obj_parser_t *parser, size_t num_vertices);
static void obj_parser_collect_vertex_data(obj_parser_t *parser, vertex_t *vertices);
static void obj_parser_collect_index_data(obj_parser_t *parser, vindex_t *indices);

// -------------------------------------------------------------------------------------------------

void obj_parser_init(obj_parser_t *parser)
{
	if (parser == NULL) {
		return;
	}

	arr_init(parser->positions);
	arr_init(parser->normals);
	arr_init(parser->texcoords);
	arr_init(parser->vertices);

	parser->object_name = NULL;
	parser->material_library = NULL;
	parser->material = NULL;
}

void obj_parser_destroy(obj_parser_t *parser)
{
	if (parser == NULL) {
		return;
	}

	arr_clear(parser->positions);
	arr_clear(parser->normals);
	arr_clear(parser->texcoords);
	arr_clear(parser->vertices);

	DESTROY(parser->object_name);
	DESTROY(parser->material_library);
	DESTROY(parser->material);
}

void obj_parser_process_line(obj_parser_t *parser, const char *line)
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
	char x_str[32];
	char y_str[32];
	char z_str[32];

	string_tokenize(line, ' ', type, sizeof(type));

	if (*type == 'v') { // Vertex data (v, vt or vn)

		if (type[1] == 0) { // v = vertex position (x, y, z [, w])

			if (num_tokens < 4) { // Need at least 3 tokens for a position.
				return;
			}

			string_tokenize(NULL, ' ', x_str, sizeof(x_str));
			string_tokenize(NULL, ' ', y_str, sizeof(y_str));
			string_tokenize(NULL, ' ', z_str, sizeof(z_str));

			vec3_t position = vec3(atof(x_str), atof(y_str), atof(z_str));

			arr_push(parser->positions, position);
		}
		else if (type[1] == 't') { // vt = texture coordinate (u, v)

			if (num_tokens < 3) { // Need at least 2 tokens for a texture coordinate.
				return;
			}

			string_tokenize(NULL, ' ', x_str, sizeof(x_str));
			string_tokenize(NULL, ' ', y_str, sizeof(y_str));

			vec2_t texcoord = vec2(atof(x_str), atof(y_str));

			arr_push(parser->texcoords, texcoord);		
		}
		else if (type[1] == 'n') { // vn = vertex normal (x, y, z)

			if (num_tokens < 4) { // Need at least 3 tokens for a normal.
				return;
			}

			string_tokenize(NULL, ' ', x_str, sizeof(x_str));
			string_tokenize(NULL, ' ', y_str, sizeof(y_str));
			string_tokenize(NULL, ' ', z_str, sizeof(z_str));

			vec3_t normal = vec3(atof(x_str), atof(y_str), atof(z_str));

			arr_push(parser->normals, normal);
		}
	}
	else if (*type == 'f') { // f = face (v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3)

		obj_parser_process_face(parser, num_tokens - 1);
	}
	else if (*type == 'o') { // o = object name (submesh)

		char name[128];
		string_tokenize_end(' ', name, sizeof(name));

		// The parser doesn't currently handle multiple objecty definitions in one .obj file.
		if (!string_is_null_or_empty(parser->object_name)) {

			log_error(".obj parser", "Multiple submeshes are not handled by the parser.");
			return;
		}

		parser->object_name = string_duplicate(name);
	}
	else if (*type == 's') { // s = smooth shading
		// Smooth shading is currently not supported by the parser.
	}
	else if (string_equals(type, "mtllib")) { // mtllib = the .mtl file associated with this object

		if (!string_is_null_or_empty(parser->material_library)) {

			log_warning(".obj parser", "Multiple material libraries defined.");
			return;
		}

		char mtl[128];
		char name[128];

		string_tokenize(NULL, ' ', mtl, sizeof(mtl));
		string_get_file_name_without_extension(mtl, name, sizeof(name));

		parser->material_library = string_duplicate(name);
	}
	else if (string_equals(type, "usemtl")) { // usemtl = use a material from the .mtl file

		if (!string_is_null_or_empty(parser->material)) {

			log_warning(".obj parser", "Multiple materials used! This is currently not supported.");
			return;
		}

		char mtl[128];
		string_tokenize(NULL, ' ', mtl, sizeof(mtl));

		parser->material = string_duplicate(mtl);
	}
	else {
		// The line contains some type of data not handled by this parser, warn the user about it.
		log_warning(".obj parser", "Unhandled data line: %s.", type);
	}
}

model_t *obj_parser_create_model(obj_parser_t *parser, const char *name, const char *path)
{
	if (parser == NULL || parser->vertices.count == 0) {
		return NULL;
	}

	// Create a mesh from the vertex data.
	vertex_t vertices[parser->vertices.count];
	obj_parser_collect_vertex_data(parser, vertices);

	vindex_t indices[parser->vertices.count];
	obj_parser_collect_index_data(parser, indices);

	// Create a model structure and add the mesh to it.
	model_t *model = model_create(name, path);
	model_add_mesh(model, vertices, parser->vertices.count, indices, parser->vertices.count);

	// Add material data from the .mtl file to the mesh.
	if (!string_is_null_or_empty(parser->material_library) &&
		!string_is_null_or_empty(parser->material)) {

		mesh_t *mesh = model->meshes.items[arr_last_index(model->meshes)];

		char material_name[1000];
		snprintf(material_name, sizeof(material_name), "%s/%s",
		         parser->material_library, parser->material);

		mesh_set_material(mesh, res_get_material(material_name));
	}

	return model;
}

static void obj_parser_process_face(obj_parser_t *parser, size_t num_vertices)
{
	if (num_vertices == 0) {
		return;
	}

	// Split the vertex definitions first.
	char vertex[num_vertices][64];
	size_t i = 0;

	while (string_tokenize(NULL, ' ', vertex[i], sizeof(vertex[i]))) { i++; }

	// Each face defines multiple vertices separated by a /, make sure to parse each of them.
	for (i = 0; i < num_vertices; i++) {

		// The format supported in the current version requires all of position, texture coordinate
		// and normal to be defined. Print an error if the file doesn't define these.
		if (string_token_count(vertex[i], '/') < 3) {

			log_error(".obj parser", "Unhandled number of tokens in face definition.");
			continue;
		}

		char position_str[32];
		char texcoord_str[32];
		char normal_str[32];

		string_tokenize(vertex[i], '/', position_str, sizeof(position_str));
		string_tokenize(NULL,   '/', texcoord_str, sizeof(texcoord_str));
		string_tokenize(NULL,   '/', normal_str, sizeof(normal_str));

		int position_idx = atoi(position_str);
		int texcoord_idx = atoi(texcoord_str);
		int normal_idx = atoi(normal_str);

		// Ensure all indices are either positive (offset from start) or negative (offset from end).
		if (position_idx == 0 || texcoord_idx == 0 || normal_idx == 0) {

			log_error(".obj parser", "Invalid face/vertex data.");
			return;
		}

		// Store the vertex definition. This is converted to a renderer vertex after the entire
		// object has been parsed.
		obj_vertex_t obj_vertex = { position_idx, normal_idx, texcoord_idx };

		arr_push(parser->vertices, obj_vertex);
	}
}

static void obj_parser_collect_vertex_data(obj_parser_t *parser, vertex_t *vertices)
{
	size_t max_vertices = parser->vertices.count;

	if (max_vertices >= MAX_VERTICES) {

		log_warning(".obj parser", "Model has too many vertices, truncating...");
		max_vertices = MAX_VERTICES - 1;
	}

	for (vindex_t i = 0; i < max_vertices; i++) {

		int position = parser->vertices.items[i].position;
		int texcoord = parser->vertices.items[i].texcoord;
		int normal = parser->vertices.items[i].normal;

		// Positive indices are offset from the start of the array (1 being the first), negative
		// indices are offset from the end of the array.
		size_t position_idx = (position < 0 ? parser->positions.count + position : position - 1);
		size_t texcoord_idx = (texcoord < 0 ? parser->texcoords.count + texcoord : texcoord - 1);
		size_t normal_idx = (normal < 0 ? parser->normals.count + normal : normal - 1);

		// Validate the data and that it has been defined in the file.
		if (position_idx >= parser->positions.count ||
			texcoord_idx >= parser->texcoords.count ||
			normal_idx >= parser->normals.count) {

			log_warning(".obj parser", "Vertex data out od bounds.");
			continue;
		}

		// Create the renderer vertex and add it to the vertex array.
		vertices[i] = vertex(
			parser->positions.items[position_idx],
			parser->normals.items[normal_idx],
			parser->texcoords.items[texcoord_idx]
		);
	}
}

static void obj_parser_collect_index_data(obj_parser_t *parser, vindex_t *indices)
{
	size_t max_vertices = parser->vertices.count;

	// Truncate too large meshes.
	if (max_vertices >= MAX_VERTICES) {
		max_vertices = MAX_VERTICES - 1;
	}

	for (vindex_t i = 0; i < max_vertices; i++) {
		indices[i] = i;
	}
}
