#include "resources.h"
#include "emitterparser.h"
#include "core/string.h"
#include "scene/emitter.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

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
		else if (res_parser_field_equals(&parser->parser, token, "emitters", JSMN_ARRAY)) {

			// TODO: Add support for subemitters!
			if (!arr_is_empty(parser->emitters)) {

				log_warning("Resources", "Multiple emitters is currently not supported "
					" (resource file %s).", path);

				return true;
			}

			// Skip the start of the array and move right on to the first subemitter.
			token += 2;

			// Loop for as long as there are (emitter) objects in the array.
			while (res_parser_is_object(&parser->parser, token)) {

				++token;

				// Create an empty emitter into which to load the parsed data.
				char name[260];
				string_get_file_name_without_extension(path, name, sizeof(name));

				emitter_t *emitter = emitter_create_from_resource(name, path);

				if (!emitter_parser_load_subemitter(parser, &token, emitter)) {

					// If parsing the emitter obquitject failed, destroy it.
					emitter_destroy(emitter);
				}
				else {
					// Add the emitter to the parsed list.
					arr_push(parser->emitters, emitter);
				}
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

static bool emitter_parser_load_subemitter(emitter_parser_t *parser, int *next_token,
                                           emitter_t *emitter)
{
	char sprite_name[100] = { 0 };
	int token = *next_token;

	emit_shape_t shape = shape_box(vec3_zero(), vec3_zero());
	emit_shape_type_t shape_type = SHAPE_POINT;

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

		else if (res_parser_field_equals(&parser->parser, token, "velocity_min", JSMN_ARRAY)) {

			emitter_set_particle_velocity(emitter,
				res_parser_get_vector(&parser->parser, (++token, &token)),
				emitter->velocity.max
			);
		}

		else if (res_parser_field_equals(&parser->parser, token, "velocity_max", JSMN_ARRAY)) {

			emitter_set_particle_velocity(emitter,
				emitter->velocity.min,
				res_parser_get_vector(&parser->parser, (++token, &token))
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

		else if (res_parser_field_equals(&parser->parser, token, "emit_centre", JSMN_ARRAY)) {

			shape.point.centre = res_parser_get_vector(&parser->parser, (++token, &token));
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_radius", JSMN_PRIMITIVE)) {

			shape.circle.radius = res_parser_get_float(&parser->parser, ++token);
			shape_type = (shape.circle.radius > 0 ? SHAPE_CIRCLE : SHAPE_POINT);
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_box_min", JSMN_ARRAY)) {

			shape.box.min = res_parser_get_vector(&parser->parser, (++token, &token));
			shape_type = SHAPE_BOX;
		}

		else if (res_parser_field_equals(&parser->parser, token, "emit_box_max", JSMN_ARRAY)) {

			shape.box.max = res_parser_get_vector(&parser->parser, (++token, &token));
			shape_type = SHAPE_BOX;
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

	// TODO: Validate everything!
	// Do it in emitter setters
	// Add setters for max particles, emit rate etc

	// Set emit shape.
	emitter_set_emit_shape(emitter, shape_type, shape);

	*next_token = token;
	return true;
}
