#pragma once
#ifndef __CONFIG_H
#define __CONFIG_H

void config_initialize(const char *directory);
void config_shutdown(void);

void config_execute_commands(void);

const char *config_get_string(const char *key, const char *default_value);
int config_get_int(const char *key, int default_value);

#endif
