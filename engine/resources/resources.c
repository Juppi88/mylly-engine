#include "resources.h"
#include "collections/array.h"
#include "core/string.h"
#include "io/file.h"
#include "io/log.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

// --------------------------------------------------------------------------------

static arr_t(texture_t*) textures;
static arr_t(shader_t*) shaders;

// --------------------------------------------------------------------------------

static void res_load_all_in_directory(const char *path, const char *extension, res_type_t type);
static void res_load_texture(const char *file_name);
static void res_load_shader(const char *file_name);
static void res_parse_shader_line(char *line, size_t length, void *context);

// --------------------------------------------------------------------------------

void res_initialize(void)
{
	// Create default resources here, add them to the beginning of resource list.
	const char *source[] = {
		NULL, // Empty line for shader resources
		rend_get_default_shader_source()
	};

	shader_t *default_shader = shader_create("default", NULL);
	shader_load_from_source(default_shader, source, 2);

	arr_push(shaders, default_shader);
	default_shader->resource.index = arr_last_index(shaders);

	// Load custom resources.
	res_load_all_in_directory("./textures", ".png", RES_TEXTURE);
	res_load_all_in_directory("./shaders", ".glsl", RES_SHADER);
}

void res_shutdown(void)
{
	// Unload all resources.
	shader_t *shader;
	arr_foreach(shaders, shader) {

		if (shader != NULL) {

			// TODO: Add reference counting to resources.
			shader_destroy(shader);
		}
	}

	texture_t *texture;
	arr_foreach(textures, texture) {

		if (texture != NULL) {

			// TODO: Add reference counting to resources.
			texture_destroy(texture);
		}
	}
}

// TODO: Load unloaded resources when requested.

texture_t *res_get_texture(const char *name)
{
	texture_t *texture;
	arr_foreach(textures, texture) {

		if (string_equals(texture->resource.name, name)) {

			// TODO: Add reference counting to resources.
			return texture;
		}
	}

	log_warning("Resources", "Could not find a texture named '%s'.", name);

	return NULL;
}

shader_t *res_get_shader(const char *name)
{
	shader_t *shader;
	arr_foreach(shaders, shader) {

		if (string_equals(shader->resource.name, name)) {

			// TODO: Add reference counting to resources.
			return shader;
		}
	}

	log_warning("Resources", "Could not find a shader named '%s'.", name);

	return NULL;
}

static void res_load_all_in_directory(const char *path, const char *extension, res_type_t type)
{
	switch (type) {

		case RES_TEXTURE:
			file_for_each_in_directory(path, extension, res_load_texture);
			break;

		case RES_SHADER:
			file_for_each_in_directory(path, extension, res_load_shader);
			break;
	}
	
}

static void res_load_texture(const char *file_name)
{
	void *buffer;
	size_t length;

	if (file_read_all_data(file_name, &buffer, &length)) {

		// Create the texture.
		char name[260];
		string_get_file_name_without_extension(file_name, name, sizeof(name));

		texture_t *texture = texture_create(name, file_name);

		if (texture_load_png(texture, buffer, length)) {
			texture->resource.is_loaded = true;
		}

		// Add to resource list.
		arr_push(textures, texture);
		texture->resource.index = arr_last_index(textures);
	}
}

static void res_load_shader(const char *file_name)
{
	// Read the shader source into an array of lines.
	arr_t(char*) lines;
	arr_init(lines);

	// Add an empty line for defines. This is a requirement for the shader compiler.
	arr_push(lines, NULL);

	if (!file_for_each_line(file_name, res_parse_shader_line, &lines, true)) {
		return;
	}

	// Create and parse the shader from the lines.
	char name[260];
	string_get_file_name_without_extension(file_name, name, sizeof(name));

	shader_t *shader = shader_create(name, file_name);

	if (shader_load_from_source(shader, (const char **)lines.items, lines.count)) {
		shader->resource.is_loaded = true;
	}

	// Add to resource list.
	arr_push(shaders, shader);
	shader->resource.index = arr_last_index(shaders);

	// Remove all temporarily allocated memory.
	char *line;

	arr_foreach(lines, line) {
		mem_free(line);
	}

	arr_clear(lines);
}

static void res_parse_shader_line(char *line, size_t length, void *context)
{
	arr_t(char*) *lines = context;

	// Check the source code for #pragma include directives.
	if (string_starts_with(line, "#pragma include ", 16)) {

		// Attempt to read the include file.
		char *include = &line[16];
		char file_name[260];

		// Strip white space from the end of the file name (line change etc).
		string_strip_end(&include);

		snprintf(file_name, sizeof(file_name), "./shaders/%s", include);

		char *include_buffer;
		size_t include_length;

		if (file_read_all_text(file_name, &include_buffer, &include_length)) {

			// Push the contents of the include file to the line list.
			arr_push(*lines, string_duplicate(include_buffer));
		}
		else {
			log_warning("Resources", "Could not include a shader named '%s'.", include);
		}
	}
	else {
		// Push the line to the list as-is.
		arr_push(*lines, string_duplicate(line));
	}
}
