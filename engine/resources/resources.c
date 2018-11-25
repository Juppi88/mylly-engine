#include "resources.h"
#include "collections/array.h"
#include "core/string.h"
#include "io/file.h"
#include "io/log.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "renderer/sprite.h"
#include "renderer/shader.h"
#include "jsmn/jsmn.h"
#include "math/math.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static arr_t(texture_t*) textures;
static arr_t(sprite_t*) sprites;
static arr_t(shader_t*) shaders;

// -------------------------------------------------------------------------------------------------

static void res_load_all_in_directory(const char *path, const char *extension, res_type_t type);
static void res_load_texture(const char *file_name);
static void res_load_sprite_sheet(const char *file_name);
static void res_load_sprite(texture_t *texture, int pixels_per_unit,
                            const char *text, jsmntok_t *tokens, size_t num_tokens, int *idx);
static void res_load_shader(const char *file_name);
static void res_parse_shader_line(char *line, size_t length, void *context);

// JSON formatted resource file parsing.
static char *res_get_text(jsmntok_t *token, const char *src, char *dst, size_t dst_len);
static int res_get_int(jsmntok_t *token, const char *src);
static bool res_get_bool(jsmntok_t *token, const char *src);
static float res_get_float(jsmntok_t *token, const char *src);

// -------------------------------------------------------------------------------------------------

void res_initialize(void)
{
	// Create default resources here (for resource types which are applicable).
	const char *source[] = {
		NULL, // Leave an empty line for shader defines.
		rend_get_default_shader_source()
	};

	shader_t *default_shader = shader_create("default", NULL);
	shader_load_from_source(default_shader, source, 2);

	arr_push(shaders, default_shader);
	default_shader->resource.index = arr_last_index(shaders);

	// Load custom resources.
	res_load_all_in_directory("./shaders", ".glsl", RES_SHADER);
	res_load_all_in_directory("./textures", ".png", RES_TEXTURE);
	res_load_all_in_directory("./textures", ".sprite", RES_SPRITE);
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

	sprite_t *sprite;
	arr_foreach(sprites, sprite) {

		if (sprite != NULL) {

			// TODO: Add reference counting to resources.
			sprite_destroy(sprite);
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

sprite_t *res_get_sprite(const char *name)
{
	sprite_t *sprite;
	arr_foreach(sprites, sprite) {

		if (string_equals(sprite->resource.name, name)) {

			// TODO: Add reference counting to resources.
			return sprite;
		}
	}

	log_warning("Resources", "Could not find a sprite named '%s'.", name);

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

		case RES_SPRITE:
			file_for_each_in_directory(path, extension, res_load_sprite_sheet);
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

		// Add the entire texture as a sprite as well (for easy 1-sprite sheet importing).
		if (texture->resource.is_loaded) {

			// Create a sprite and set its data.
			sprite_t *sprite = sprite_create(name);
			sprite_set(sprite, texture,
				vec2_zero, vector2(texture->width, texture->height), vec2_zero, 100);

			// Add to resource list.
			sprite->resource.index = arr_last_index(sprites);
			sprite->resource.is_loaded = true;

			arr_push(sprites, sprite);
		}
	}
}

static void res_load_sprite_sheet(const char *file_name)
{
	char *text;
	size_t length;

	char name[260];
	string_get_file_name_without_extension(file_name, name, sizeof(name));

	// Make sure a texture for this spritesheet exists.
	texture_t *texture = res_get_texture(name);

	if (texture == NULL) {

		log_warning("Resources", "Could not find a texture for spritesheet '%s'.", name);
		return;
	}

	// Read the contents of the .sprite file to a buffer the file is JSON formatted data).
	if (!file_read_all_text(file_name, &text, &length)) {
		return;
	}

	// Initialize a JSON parser object.
	jsmn_parser parser;
	jsmn_init(&parser);

	// Parse the text from the file.
	const int MAX_TOKENS = 256;
	jsmntok_t tokens[MAX_TOKENS];

	int num_tokens = jsmn_parse(&parser, text, length, tokens, MAX_TOKENS);

	// Negative token count indicates an error -> not a valid resource file.
	if (num_tokens < 0) {
		return;
	}

	// Assume the top-level element is always an object.
	if (num_tokens == 0 || tokens[0].type != JSMN_OBJECT) {
		return;
	}

	int pixels_per_unit = 100;
	bool sprites_defined = false;

	for (int i = 1; i < num_tokens; i++) {

		// Key should always be a string or a primitive!
		if (tokens[i].type != JSMN_STRING && tokens[i].type != JSMN_PRIMITIVE) {
			continue;
		}

		char key[100];
		
		// Get the key (name of the field).
		res_get_text(&tokens[i], text, key, sizeof(key));

		if (string_equals(key, "version") && tokens[i+1].type == JSMN_PRIMITIVE) {

			// Ignore version checks for now (they're for future proofing).
			++i;
		}
		else if (string_equals(key, "pixels_per_unit") && tokens[i+1].type == JSMN_PRIMITIVE) {

			// Define sheet-wide pixels per unit.
			pixels_per_unit = res_get_int(&tokens[++i], text);

			if (sprites_defined) {
				log_warning(
					"Resources",
					"pixels_per_unit should be defined before the sprites (%s)",
				 	file_name);
			}
		}
		else if (string_equals(key, "sprites") && tokens[i+1].type == JSMN_ARRAY) {

			// Skip the start of the array and move right on to the first sprite.
			i += 2;

			// Loop for as long as there are sprite objects in the array.
			while (tokens[i++].type == JSMN_OBJECT && i < num_tokens) {
				res_load_sprite(texture, pixels_per_unit, text, tokens, num_tokens, &i);
			}

			--i; // Negate the increment in each iteration.
			sprites_defined = true;
		}
		else {
			// Unknown field name or type, skip it (unless it's an object of unknown size => abort).
			log_warning("Resources", "Unknown sprite field '%s' in resource file %s.",
				key, file_name);

			++i;
			if (tokens[i].type != JSMN_PRIMITIVE && tokens[i].type != JSMN_STRING) {
				return;
			}
		}
	}
}

static void res_load_sprite(texture_t *texture, int pixels_per_unit,
                            const char *text, jsmntok_t *tokens, size_t num_tokens, int *idx)
{
	char name[100];
	vec2_t position = vec2_zero;
	vec2_t size = vec2_zero;
	vec2_t pivot = vec2_zero;
	bool flip_vertical = false;
	int i = 0;

	for (i = *idx; i < num_tokens; ++i) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the sprite object and should return to the main parser.
		if (tokens[i].type != JSMN_STRING && tokens[i].type != JSMN_PRIMITIVE) {
			break;
		}

		// Get the name of the field.
		char key[100];
		res_get_text(&tokens[i], text, key, sizeof(key));

		// Read the value based on the name of the field.
		if (string_equals(key, "name") && tokens[i+1].type == JSMN_STRING) {
			res_get_text(&tokens[++i], text, name, sizeof(name));
		}
		else if (string_equals(key, "pos_x") && tokens[i+1].type == JSMN_PRIMITIVE) {
			position.x = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "pos_y") && tokens[i+1].type == JSMN_PRIMITIVE) {
			position.y = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "width") && tokens[i+1].type == JSMN_PRIMITIVE) {
			size.x = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "height") && tokens[i+1].type == JSMN_PRIMITIVE) {
			size.y = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "pivot_x") && tokens[i+1].type == JSMN_PRIMITIVE) {
			pivot.x = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "pivot_y") && tokens[i+1].type == JSMN_PRIMITIVE) {
			pivot.y = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "pixels_per_unit") && tokens[i+1].type == JSMN_PRIMITIVE) {
			pixels_per_unit = res_get_int(&tokens[++i], text);
		}
		else if (string_equals(key, "flip_y") && tokens[i+1].type == JSMN_PRIMITIVE) {
			flip_vertical = res_get_bool(&tokens[++i], text);
		}
		else {
			// Unknown field, return to main parser.
			break;
		}
	}

	// Create a sprite object and store it to the resource list.
	if (*name) {

		// Flip vertical position if it is measured from the top of the sheet instead of the bottom.
		if (flip_vertical) {
			position.y = texture->height - position.y;
		}

		// Sprites have formatted names such as <sheet name>/<sprite name>.
		char sprite_name[200];
		snprintf(sprite_name, sizeof(sprite_name), "%s/%s", texture->resource.name, name);

		// Create a sprite and set its data.
		sprite_t *sprite = sprite_create(sprite_name);
		sprite_set(sprite, texture, position, size, pivot, pixels_per_unit);

		// Add to resource list.
		sprite->resource.index = arr_last_index(sprites);
		sprite->resource.is_loaded = true;

		arr_push(sprites, sprite);
	}

	*idx = i;
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

static char *res_get_text(jsmntok_t *token, const char *src, char *dst, size_t dst_len)
{
	dst_len = MIN(dst_len, token->end - token->start + 1);
	string_copy(dst, &src[token->start], dst_len);

	return dst;
}

static int res_get_int(jsmntok_t *token, const char *src)
{
	char tmp[100];
	size_t length = MIN(sizeof(tmp), token->end - token->start + 1);

	string_copy(tmp, &src[token->start], length);

	return atoi(tmp);
}

static bool res_get_bool(jsmntok_t *token, const char *src)
{
	char tmp[100];
	size_t length = MIN(sizeof(tmp), token->end - token->start + 1);

	string_copy(tmp, &src[token->start], length);

	return string_equals(tmp, "true");
}

static float res_get_float(jsmntok_t *token, const char *src)
{
	char tmp[100];
	size_t length = MIN(sizeof(tmp), token->end - token->start + 1);

	string_copy(tmp, &src[token->start], length);

	return atof(tmp);
}
