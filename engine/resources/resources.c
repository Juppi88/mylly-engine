#include "resources.h"
#include "resourceparser.h"
#include "collections/array.h"
#include "core/string.h"
#include "io/file.h"
#include "io/log.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include "scene/sprite.h"
#include "scene/spriteanimation.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

static arr_t(texture_t*) textures;
static arr_t(sprite_t*) sprites;
static arr_t(shader_t*) shaders;
static arr_t(sprite_anim_t*) animations;

static const char *parsed_file_name;

// -------------------------------------------------------------------------------------------------

static void res_load_all_in_directory(const char *path, const char *extension, res_type_t type);

static void res_load_texture(const char *file_name);

static void res_load_sprite_sheet(const char *file_name);
static void res_load_sprite(texture_t *texture, int pixels_per_unit,
                            res_parser_t *parser, int *next_token);

static void res_load_shader(const char *file_name);
static void res_parse_shader_line(char *line, size_t length, void *context);

static void res_load_animation_group(const char *file_name);
static void res_load_animation(res_parser_t *parser, int *next_token, const char *group_name);
static void res_load_animation_keyframe(res_parser_t *parser, int *next_token,
                                        sprite_t **sprite, int *frame_count);

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
	res_load_all_in_directory("./animations", ".anim", RES_ANIMATION);
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

	sprite_anim_t *animation;
	arr_foreach(animations, animation) {

		if (animation != NULL) {

			// TODO: Add reference counting to resources.
			sprite_anim_destroy(animation);
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

sprite_anim_t *res_get_sprite_anim(const char *name)
{
	sprite_anim_t *animation;
	arr_foreach(animations, animation) {

		if (string_equals(animation->resource.name, name)) {

			// TODO: Add reference counting to resources.
			return animation;
		}
	}

	log_warning("Resources", "Could not find a sprite animation named '%s'.", name);

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

		case RES_ANIMATION:
			file_for_each_in_directory(path, extension, res_load_animation_group);
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
	// Store the name of the file for possible error messages.
	parsed_file_name = file_name;
	
	char name[260];
	string_get_file_name_without_extension(file_name, name, sizeof(name));

	// Make sure a texture for this spritesheet exists.
	texture_t *texture = res_get_texture(name);

	if (texture == NULL) {

		log_warning("Resources", "Could not find a texture for spritesheet '%s'.", name);
		return;
	}

	char *text;
	size_t length;

	// Read the contents of the .sprite file to a buffer.
	if (!file_read_all_text(file_name, &text, &length)) {
		return;
	}

	// Initialize a resource parser object.
	res_parser_t parser;
	
	// Parse the text from the file.
	if (!res_parser_init(&parser, text, length)) {

		// Resource could not be parsed (syntax error or not a JSON resource file).
		mem_free(text);
		return;
	}
	
	int pixels_per_unit = 100;
	bool sprites_defined = false;

	for (int token = 1; token < parser.num_tokens; token++) {

		// Key should always be a string or a primitive!
		if (!res_parser_is_valid_key_type(&parser, token)) {
			continue;
		}

		// Process each field based on its name and value type.
		if (res_parser_field_equals(&parser, token, "version", JSMN_PRIMITIVE)) {

			// Ignore version checks for now (they're for future proofing).
			++token;
		}
		else if (res_parser_field_equals(&parser, token, "pixels_per_unit", JSMN_PRIMITIVE)) {

			// Define sheet-wide pixels per unit.
			pixels_per_unit = res_parser_get_int(&parser, ++token);

			if (sprites_defined) {
				log_warning(
					"Resources",
					"pixels_per_unit should be defined before the sprites (%s)",
				 	file_name);
			}
		}
		else if (res_parser_field_equals(&parser, token, "sprites", JSMN_ARRAY)) {

			// Skip the start of the array and move right on to the first sprite.
			token += 2;

			// Loop for as long as there are sprite objects in the array.
			while (res_parser_is_object(&parser, token)) {

				++token;
				res_load_sprite(texture, pixels_per_unit, &parser, &token);
			}

			sprites_defined = true;
		}
		else {
			// Unknown field name or type, skip it (unless it's an object of unknown size => abort).
			char key[100];
			res_parser_get_text(&parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown sprite field '%s' in resource file %s.",
				key, file_name);

			if (!res_parser_is_valid_key_type(&parser, ++token)) {
				break;
			}
		}
	}

	// Free the temporary file content buffer.
	mem_free(text);
}

static void res_load_sprite(texture_t *texture, int pixels_per_unit,
                            res_parser_t *parser, int *next_token)
{
	char name[100] = { 0 };
	vec2_t position = vec2_zero;
	vec2_t size = vec2_zero;
	vec2_t pivot = vec2_zero;
	bool flip_vertical = false;
	int token = *next_token;

	for (; token < parser->num_tokens; ++token) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the sprite object and should return to the main parser.
		if (!res_parser_is_valid_key_type(parser, token)) {
			break;
		}

		// Read the value based on the name and type of the field.
		if (res_parser_field_equals(parser, token, "name", JSMN_STRING)) {
			res_parser_get_text(parser, ++token, name, sizeof(name));
		}
		else if (res_parser_field_equals(parser, token, "pos_x", JSMN_PRIMITIVE)) {
			position.x = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "pos_y", JSMN_PRIMITIVE)) {
			position.y = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "width", JSMN_PRIMITIVE)) {
			size.x = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "height", JSMN_PRIMITIVE)) {
			size.y = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "pivot_x", JSMN_PRIMITIVE)) {
			pivot.x = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "pivot_y", JSMN_PRIMITIVE)) {
			pivot.y = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "pixels_per_unit", JSMN_PRIMITIVE)) {
			pixels_per_unit = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "flip_y", JSMN_PRIMITIVE)) {
			flip_vertical = res_parser_get_bool(parser, ++token);
		}
		else {
			// Unknown field, return to main parser.
			char key[100];
			res_parser_get_text(parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown sprite field '%s' in resource file %s.",
                        key, parsed_file_name);

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

	*next_token = token;
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

static void res_load_animation_group(const char *file_name)
{
	// Store the name of the file for possible error messages.
	parsed_file_name = file_name;


	// Use the name of the file as the animation group name.
	// The group name is used as a prefix for all animations.
	char group_name[260];
	string_get_file_name_without_extension(file_name, group_name, sizeof(group_name));

	char *text;
	size_t length;

	// Read the contents of the .anim file to a buffer.
	if (!file_read_all_text(file_name, &text, &length)) {
		return;
	}

	// Initialize a resource parser object.
	res_parser_t parser;
	
	// Parse the text from the file.
	if (!res_parser_init(&parser, text, length)) {

		// Resource could not be parsed (syntax error or not a JSON resource file).
		mem_free(text);
		return;
	}

	for (int token = 1; token < parser.num_tokens; token++) {

		// Key should always be a string or a primitive!
		if (!res_parser_is_valid_key_type(&parser, token)) {
			continue;
		}

		// Process each field based on its name and value type.
		if (res_parser_field_equals(&parser, token, "version", JSMN_PRIMITIVE)) {

			// Ignore version checks for now (they're for future proofing).
			++token;
		}
		else if (res_parser_field_equals(&parser, token, "animation", JSMN_OBJECT)) {

			// Skip to the first child field of the token and parse an animation object.
			token += 2;
			res_load_animation(&parser, &token, group_name);
		}
		else {
			// Unknown field name or type, skip it (unless it's an object of unknown size => abort).
			char key[100];
			res_parser_get_text(&parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown animation group field '%s' in resource file %s.",
				key, file_name);

			if (!res_parser_is_valid_key_type(&parser, ++token)) {
				break;
			}
		}
	}

	// Free the temporary file content buffer.
	mem_free(text);
}

static void res_load_animation(res_parser_t *parser, int *next_token, const char *group_name)
{
	char name[100] = { 0 };
	int anim_duration = 0;
	int token = *next_token;

	// Create a temporary container for the keyframes.
	arr_t(keyframe_t) keyframes;
	arr_init(keyframes);

	for (; token < parser->num_tokens; ++token) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the sprite object and should return to the main parser.
		if (!res_parser_is_valid_key_type(parser, token)) {
			break;
		}

		// Read the value based on the name and type of the field.
		if (res_parser_field_equals(parser, token, "name", JSMN_STRING)) {
			res_parser_get_text(parser, ++token, name, sizeof(name));
		}
		else if (res_parser_field_equals(parser, token, "duration", JSMN_PRIMITIVE)) {
			anim_duration = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_field_equals(parser, token, "keyframes", JSMN_ARRAY)) {

			// Skip the start of the array and move right on to the first keyframe.
			token += 2;

			// Loop for as long as there are keyframe objects in the array.
			while (res_parser_is_object(parser, token)) {

				++token;

				sprite_t *sprite = NULL;
				int frames = 0;

				res_load_animation_keyframe(parser, &token, &sprite, &frames);

				// Add the keyframe to the list.
				if (sprite != NULL) {

					keyframe_t keyframe;
					keyframe.sprite = sprite;
					keyframe.duration = frames;

					arr_push(keyframes, keyframe);
				}
			}
		}
		else if (res_parser_is_object(parser, token + 1)) {

			// The following token is another object (either a keyframe or an animation).
			// Skip it and decrement the current token index due to the increment in this loop.
			--token;
			break;
		}
		else {
			// Unknown field, return to main parser.
			char key[100];
			res_parser_get_text(parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown animation field '%s' in resource file %s.",
                        key, parsed_file_name);

			break;
		}
	}

	// Create a sprite object and store it to the resource list.
	if (*name) {

		if (keyframes.count != 0) {

			// Animations have names formatted as <group name>/<animation name>.
			char animation_name[200];
			snprintf(animation_name, sizeof(animation_name), "%s/%s", group_name, name);

			// Create an animation and set its keyframes..
			sprite_anim_t *animation = sprite_anim_create(animation_name);
			
			if (sprite_anim_set_frames(animation, keyframes.items, keyframes.count, anim_duration)) {

				// If the keyframes were set successfully, add the animation to the resource list.
				animation->resource.index = arr_last_index(animations);
				animation->resource.is_loaded = true;

				arr_push(animations, animation);
			}
			else {
				// Keyframe setting failed, perform cleanup.
				sprite_anim_destroy(animation);
			}
		}
		else {
			log_warning("Resources", "Sprite animation %s in file %s does not define any keyframes.",
                        name, parsed_file_name);
		}
	}

	arr_clear(keyframes);

	*next_token = token;
}

static void res_load_animation_keyframe(res_parser_t *parser, int *next_token,
                                        sprite_t **sprite, int *frame_count)
{
	char sprite_name[100] = { 0 };
	int frames = 0;
	int token = *next_token;

	for (; token < parser->num_tokens; ++token) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the keyframe object and should return to the main parser.
		if (!res_parser_is_valid_key_type(parser, token)) {
			break;
		}

		// Read the value based on the name and type of the field.
		if (res_parser_field_equals(parser, token, "sprite", JSMN_STRING)) {
			res_parser_get_text(parser, ++token, sprite_name, sizeof(sprite_name));
		}
		else if (res_parser_field_equals(parser, token, "frames", JSMN_PRIMITIVE)) {
			frames = res_parser_get_int(parser, ++token);
		}
		else if (res_parser_is_object(parser, token + 1)) {

			// The following token is another object (either a keyframe or an animation).
			// Skip it and decrement the current token index due to the increment in this loop.
			--token;
			break;
		}
		else {
			// Unknown field, return to main parser.
			char key[100];
			res_parser_get_text(parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown animation keyframe field '%s' in resource file %s.",
                        key, parsed_file_name);

			break;
		}
	}

	if (*sprite_name) {

		// Pass the sprite and frame duration back to the caller.
		*sprite = res_get_sprite(sprite_name);
		*frame_count = frames;
	}

	*next_token = token;
}
