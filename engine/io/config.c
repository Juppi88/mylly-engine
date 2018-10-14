#include "config.h"
#include "console.h"
#include "log.h"
#include "file.h"
#include "collections/stack.h"
#include "core/memory.h"
#include "core/string.h"
#include <stdio.h>
#include <stdlib.h>

// --------------------------------------------------------------------------------

typedef struct setting_t {
	
	stack_entry(setting_t);

	char *key;
	char *value;

} setting_t;

static stack_t(setting_t) settings;
static stack_t(setting_t) commands;

// --------------------------------------------------------------------------------

static void config_parse_file(const char *path);
static void config_parse_line(char *line, size_t line_len, void *context);
static setting_t *config_get_setting(const char *key);

// --------------------------------------------------------------------------------

void config_initialize(const char *directory)
{
	file_for_each_in_directory(directory, ".conf", config_parse_file);
}

void config_shutdown(void)
{
	stack_foreach_safe(setting_t, setting, settings) {
		stack_foreach_safe_begin(setting);

		mem_free(setting->key);
		mem_free(setting->value);
		mem_free(setting);
	}
}

void config_execute_commands(void)
{
	stack_foreach_safe(setting_t, command, commands) {
		stack_foreach_safe_begin(command);

		// Execute the command...
		console_execute_command(command->key, command->value);

		// ...and delete it from the queue.
		mem_free(command->key);
		mem_free(command->value);
		mem_free(command);
	}
}

const char *config_get_string(const char *key, const char *default_value)
{
	if (key != NULL) {
		setting_t *setting = config_get_setting(key);

		if (setting != NULL) {
			return setting->value;
		}
	}

	return default_value;
}

int config_get_int(const char *key, int default_value)
{
	if (key != NULL) {
		setting_t *setting = config_get_setting(key);

		if (setting != NULL) {
			return atoi(setting->value);
		}
	}

	return default_value;
}

static void config_parse_file(const char *path)
{
	file_for_each_line(path, config_parse_line, NULL, false);
}

static void config_parse_line(char *line, size_t line_len, void *context)
{
	char *key, *value;
	string_parse_command(line, &key, &value);

	if (string_is_null_or_empty(key)) {
		return;
	}

	// Lines prefixed with a ! are to be parsed as commands after all subsystems have initialized.
	if (*key == '!') {

		if (*++key != 0) {

			NEW(setting_t, command);

			command->key = string_duplicate(key);
			command->value = string_duplicate(value);

			stack_push(commands, command);
		}

		return;
	}

	// Make sure existing config keys aren't overriden.
	if (config_get_setting(key) != NULL) {

		log_error("config", "Found a duplicate value for key '%s'!", key);
		return;
	}

	// Add the setting entry to the list.
	NEW(setting_t, setting);

	setting->key = string_duplicate(key);
	setting->value = string_duplicate(value);

	stack_push(settings, setting);
}

static setting_t *config_get_setting(const char *key)
{
	stack_foreach(setting_t, setting, settings) {

		if (string_equals(setting->key, key)) {
			return setting;
		}
	}

	return NULL;
}
