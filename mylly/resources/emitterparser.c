#include "resources.h"
#include "emitterparser.h"
#include "core/string.h"
#include "scene/emitter.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static bool emitter_parser_load_data(emitter_parser_t *parser, int *next_token,
                                     emitter_t *emitter);

static bool emitter_parser_load_subemitter(emitter_parser_t *parser, int *next_token,
                                           emitter_t *emitter);

// -------------------------------------------------------------------------------------------------

static const char *parsed_file_name;

// -------------------------------------------------------------------------------------------------

bool emitter_parser_init(emitter_parser_t *parser, const char *path,
                         const char *contents, size_t contents_length)
{
	parsed_file_name = path;

	arr_init(parser->emitters);

	// Initialize the JSON resource parser.
	if (!res_parser_init(&parser->parser, contents, contents_length)) {

		log_warning("Resources", "Could not parse effect file %s.", path);
		return false;
	}

	for (int token = 1; token < parser->parser.num_tokens; token++) {

		// Key should always be a string or a primitive!
		if (!res_parser_is_valid_key_type(&parser->parser, token)) {
			continue;
		}

		// Process each field based on its name and value type.
		if (res_parser_field_equals(&parser->parser, token, "version", JSMN_PRIMITIVE)) {

			// Ignore version checks for now (they're for future proofing).
			++token;
		}
		else if (res_parser_field_equals(&parser->parser, token, "emitter", JSMN_OBJECT)) {

			// Skip the starting tokens of the object.
			token += 2;

			// Create an empty emitter into which to load the parsed data.
			char name[260];
			string_get_file_name_without_extension(path, name, sizeof(name));

			emitter_t *emitter = emitter_create_from_resource(name, path);

			if (!emitter_parser_load_data(parser, &token, emitter)) {

				// If parsing the emitter obquitject failed, destroy it.
				emitter_destroy(emitter);
			}
			else {
				// Add the emitter to the parsed list.
				arr_push(parser->emitters, emitter);
			}
		}
		else {
			// Unknown field name or type, skip it (unless it's an object of unknown size => abort).
			char key[100];
			res_parser_get_text(&parser->parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown emitter field '%s' in resource file %s.",
				key, path);

			if (!res_parser_is_valid_key_type(&parser->parser, ++token)) {
				break;
			}
		}
	}

	return !arr_is_empty(parser->emitters);
}

void emitter_parser_destroy(emitter_parser_t *parser)
{
	arr_clear(parser->emitters);
}

void emitter_parser_end_file(emitter_parser_t *parser)
{
	// Nothing to do here.
	UNUSED(parser);
}

static bool emitter_parser_load_data(emitter_parser_t *parser, int *next_token,
                                           emitter_t *emitter)
{
	char sprite_name[100] = { 0 };
	int token = *next_token;

	emit_shape_t shape = shape_circle(vec3_zero(), 0);

	for (; token < parser->parser.num_tokens; ++token) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the parsed object and should return to the main parser.
		if (!res_parser_is_valid_key_type(&parser->parser, token)) {
			break;
		}

		// Read the value based on the name and type of the field.
		if (res_parser_field_equals(&parser->parser, token, "sprite", JSMN_STRING)) {

			res_parser_get_text(&parser->parser, ++token, sprite_name, sizeof(sprite_name));
			sprite_t *sprite = res_get_sprite(sprite_name);

			if (sprite != NULL) {
				emitter_set_particle_sprite(emitter, sprite);
			}
		}

		else if (res_parser_field_equals(&parser->parser, token, "is_world_space", JSMN_PRIMITIVE)) {

			emitter_set_world_space(emitter, res_parser_get_bool(&parser->parser, ++token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "max_particles", JSMN_PRIMITIVE)) {

			emitter_set_max_particles(emitter, res_parser_get_int(&parser->parser, ++token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_duration", JSMN_PRIMITIVE)) {

			emitter_set_emit_duration(emitter, res_parser_get_float(&parser->parser, ++token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_rate", JSMN_PRIMITIVE)) {

			emitter_set_emit_rate(emitter, res_parser_get_float(&parser->parser, ++token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "initial_burst", JSMN_PRIMITIVE)) {

			emitter_set_initial_burst(emitter, res_parser_get_int(&parser->parser, ++token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "life_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_life_time(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->life.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "life_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_life_time(emitter,
				emitter->life.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_size_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_start_size(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->start_size.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_size_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_start_size(emitter,
				emitter->start_size.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_size_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_end_size(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->end_size.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_size_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_end_size(emitter,
				emitter->end_size.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "rotation_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_rotation_speed(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->rotation_speed.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "rotation_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_rotation_speed(emitter,
				emitter->rotation_speed.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_speed_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_start_speed(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->start_speed.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_speed_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_start_speed(emitter,
				emitter->start_speed.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_speed_min", JSMN_PRIMITIVE)) {

			emitter_set_particle_end_speed(emitter,
				res_parser_get_float(&parser->parser, ++token),
				emitter->end_speed.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_speed_max", JSMN_PRIMITIVE)) {

			emitter_set_particle_end_speed(emitter,
				emitter->end_speed.min,
				res_parser_get_float(&parser->parser, ++token)
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "acceleration_min", JSMN_ARRAY)) {

			emitter_set_particle_acceleration(emitter,
				res_parser_get_vector(&parser->parser, (++token, &token)),
				emitter->acceleration.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "acceleration_max", JSMN_ARRAY)) {

			emitter_set_particle_acceleration(emitter,
				emitter->acceleration.min,
				res_parser_get_vector(&parser->parser, (++token, &token))
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_colour_min", JSMN_ARRAY)) {

			emitter_set_particle_start_colour(emitter,
				res_parser_get_colour(&parser->parser, (++token, &token)),
				emitter->start_colour.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "start_colour_max", JSMN_ARRAY)) {

			emitter_set_particle_start_colour(emitter,
				emitter->start_colour.min,
				res_parser_get_colour(&parser->parser, (++token, &token))
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_colour_min", JSMN_ARRAY)) {

			emitter_set_particle_end_colour(emitter,
				res_parser_get_colour(&parser->parser, (++token, &token)),
				emitter->end_colour.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "end_colour_max", JSMN_ARRAY)) {

			emitter_set_particle_end_colour(emitter,
				emitter->end_colour.min,
				res_parser_get_colour(&parser->parser, (++token, &token))
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_position", JSMN_ARRAY)) {

			shape.position = res_parser_get_vector(&parser->parser, (++token, &token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_circle_radius", JSMN_PRIMITIVE)) {

			shape.type = SHAPE_CIRCLE;
			shape.circle.radius = res_parser_get_float(&parser->parser, ++token);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_sphere_radius", JSMN_PRIMITIVE)) {

			shape.type = SHAPE_SPHERE;
			shape.sphere.radius = res_parser_get_float(&parser->parser, ++token);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_box_extents", JSMN_ARRAY)) {

			shape.type = SHAPE_BOX;
			shape.box.extents = res_parser_get_vector(&parser->parser, (++token, &token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_cone_angle", JSMN_PRIMITIVE)) {

			shape.type = SHAPE_CONE;
			shape.cone.angle = res_parser_get_float(&parser->parser, ++token);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_cone_radius", JSMN_PRIMITIVE)) {

			shape.type = SHAPE_CONE;
			shape.cone.radius = res_parser_get_float(&parser->parser, ++token);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_cone_volume", JSMN_PRIMITIVE)) {

			shape.type = SHAPE_CONE;
			shape.cone.emit_volume = res_parser_get_float(&parser->parser, ++token);
		}

		else if (res_parser_field_equals(&parser->parser, token, "subemitters", JSMN_ARRAY)) {

			// Skip the starting tokens of the object.
			token += 2;

			// Loop for as long as there are (emitter) objects in the array.
			while (res_parser_is_object(&parser->parser, token)) {

				++token;
				emitter_parser_load_subemitter(parser, &token, emitter);
			}
		}

		else {
			// Unknown field, return to main parser.
			char key[100];
			res_parser_get_text(&parser->parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown emitter field '%s' in resource file %s.",
                        key, parsed_file_name);

			break;
		}
	}

	// Set emit shape.
	emitter_set_emit_shape(emitter, shape);

	*next_token = token;
	return true;
}

static bool emitter_parser_load_subemitter(emitter_parser_t *parser, int *next_token,
                                           emitter_t *emitter)
{
	char effect_name[100] = { 0 };
	subemitter_type_t type = SUBEMITTER_CREATE;

	int token = *next_token;

	for (; token < parser->parser.num_tokens; ++token) {

		// Key should always be a string or a primitive! If this is something else,
		// then likely we've passed the parsed object and should return to the main parser.
		if (!res_parser_is_valid_key_type(&parser->parser, token)) {
			break;
		}

		// Read the value based on the name and type of the field.
		if (res_parser_field_equals(&parser->parser, token, "effect", JSMN_STRING)) {

			res_parser_get_text(&parser->parser, ++token, effect_name, sizeof(effect_name));
		}

		else if (res_parser_field_equals(&parser->parser, token, "type", JSMN_PRIMITIVE)) {

			type = (subemitter_type_t)res_parser_get_int(&parser->parser, ++token);
		}

		else {
			// Unknown field, return to main parser.
			char key[100];
			res_parser_get_text(&parser->parser, token, key, sizeof(key));

			log_warning("Resources", "Unknown subemitter field '%s' in resource file %s.",
                        key, parsed_file_name);

			break;
		}
	}

	*next_token = token;

	// If the file defined an effect for the subemitter, add it to the main emitter. The actual
	// emitter resource is loaded later when all effects have been parsed.
	if (!string_is_null_or_empty(effect_name)) {

		arr_push(emitter->subemitters,
		         create_subemitter_name(type, string_duplicate(effect_name)));
		
		return true;
	}

	return false;
}
