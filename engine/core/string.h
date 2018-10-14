#pragma once
#ifndef __STRING_H
#define __STRING_H

#include "core/defines.h"
#include <string.h>

#define string_equals(s1, s2)\
	(strcmp(s1, s2) == 0)

#define string_starts_with(str, prefix, prefix_len)\
	(strncmp(prefix, str, prefix_len) == 0)

#define string_is_null_or_empty(str)\
	((str) == NULL || *(str) == 0)

#define TO_STRING(x) STRINGIFY(x)
#define STRINGIFY(x) #x
#define BOOL_STRING(x) (x ? "true" : "false")

BEGIN_DECLARATIONS;

void string_copy(char *dst, const char *src, size_t dst_len);
char *string_duplicate(const char *text);
size_t string_tokenize(const char *text, char delimiter, char *dst, size_t dst_len);
size_t string_tokenize_end(char delimiter, char *dst, size_t dst_len);
const char *string_last_token(const char *text, char delimiter);
size_t string_token_count(const char *text, char delimiter);

void string_parse_command(char *input, char **command, char **args);
void string_parse_device_id(const char *text, char delimiter, char *dst, size_t dst_len);

char *string_strip(char **input);
char *string_strip_end(char **input);

bool string_is_numeric(const char *text);

void string_get_file_name_without_extension(const char *text, char *dst, size_t dst_len);

END_DECLARATIONS;

#endif
