#pragma once
#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "core/defines.h"

typedef void (*command_handler_t)(const char *command, char *args);
#define COMMAND_HANDLER(x) static void x(const char *command, char *args)

BEGIN_DECLARATIONS;

void console_initialize(void);
void console_process(void);
void console_shutdown(void);

void console_add_command(const char *command, command_handler_t handler);
void console_execute_command(const char *command, char *args);

END_DECLARATIONS;

#endif
