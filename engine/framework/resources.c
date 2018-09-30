#include "resources.h"
#include "core/array.h"
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

// --------------------------------------------------------------------------------

void res_initialize(void)
{
	// Create default resources here, add them to the beginning of resource list.
	shader_t *default_shader = shader_create("default", NULL);
	shader_load_from_source(default_shader, rend_get_default_shader_source());

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

	log_note("Resources", "Could not find a texture named '%s'.", name);

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

	log_note("Resources", "Could not find a shader named '%s'.", name);

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
	char *buffer;
	size_t length;

	if (file_read_all_text(file_name, &buffer, &length)) {

		// Create the shader.
		char name[260];
		string_get_file_name_without_extension(file_name, name, sizeof(name));

		shader_t *shader = shader_create(name, file_name);

		if (shader_load_from_source(shader, buffer)) {
			shader->resource.is_loaded = true;
		}

		// Add to resource list.
		arr_push(shaders, shader);
		shader->resource.index = arr_last_index(shaders);
	}
}
