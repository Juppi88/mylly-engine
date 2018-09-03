#include "console.h"
#include "core/list.h"
#include "core/memory.h"
#include "core/string.h"
#include "platform/thread.h"
#include <stdio.h>

// --------------------------------------------------------------------------------

struct command_t {
	LIST_ENTRY(command_t);

	char *name;
	command_handler_t handler;
};

static char input[1024];
//static lock_t lock;

static LIST(command_t) commands;

// --------------------------------------------------------------------------------

static LIST_OBJ(command_t) console_get_command(const char *command);

// --------------------------------------------------------------------------------

THREAD(console_thread)
{
	(void)args;

	extern bool is_app_running;

	// Read input from stdin in a separate thread and safely move it to a readable buffer.
	while (is_app_running) {

//		thread_lock(&lock); {
			char *text = fgets(input, sizeof(input), stdin);
			(void)text;
//		}
//		thread_unlock(&lock);

		thread_sleep(10);
	}

	return 0;
}

void console_initialize(void)
{
	// Create a thread to read console input.
//	thread_init_lock(&lock);
	thread_create(console_thread, NULL);
}

void console_process(void)
{
//	thread_lock(&lock); {

		// Process input from the buffer.
		if (*input != 0) {
			
			char *cmd, *args;
			string_parse_command(input, &cmd, &args);

			// Call the handler method for the command if it exists.
			LIST_OBJ(command_t) command = console_get_command(cmd);

			if (command != NULL) {
				command->handler(cmd, args);
			}

			*input = 0;
		}
//	}
//	thread_unlock(&lock);
}

void console_shutdown(void)
{
	LIST_FOREACH_SAFE(command_t, cmd, commands) {
		LIST_FOREACH_SAFE_BEGIN(cmd);

		mem_free(cmd->name);
		mem_free(cmd);
	}

	// Destroy the mutex lock.
//	thread_destroy_lock(&lock);
}

void console_add_command(const char *command, command_handler_t handler)
{
	if (command == NULL || handler == NULL) {
		return;
	}

	// If the handler already exists, update its method.
	LIST_OBJ(command_t) old = console_get_command(command);

	if (old != NULL) {
		old->handler = handler;
		return;
	}

	// Create a new command handler and add it to the list.
	NEW(command_t, cmd);

	cmd->name = string_duplicate(command);
	cmd->handler = handler;

	LIST_ADD(commands, cmd);
}

void console_execute_command(const char *command, char *args)
{
	LIST_OBJ(command_t) cmd = console_get_command(command);

	if (cmd != NULL && cmd->handler != NULL) {
		cmd->handler(command, args);
	}
}

static LIST_OBJ(command_t) console_get_command(const char *command)
{
	LIST_FOREACH(command_t, cmd, commands) {

		if (string_equals(cmd->name, command)) {
			return cmd;
		}
	}

	return NULL;
}
