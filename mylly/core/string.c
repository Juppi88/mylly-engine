#include "string.h"
#include "memory.h"
#include <string.h>

static const char *tokenized_param;

void string_copy(char *dst, const char *src, size_t dst_len)
{
	while (--dst_len && *src) {
		*dst++ = *src++;
	}

	*dst = 0;
}

char *string_duplicate(const char *text)
{
	if (text == NULL) {
		return NULL;
	}

	char *buf = mem_alloc(strlen(text) + 1), *s = buf;

	while (*text) {
		*s++ = *text++;
	}

	*s = 0;
	return buf;
}

size_t string_tokenize(const char *text, char delimiter, char *dst, size_t dst_len)
{
	return string_tokenize_filter(text, delimiter, dst, dst_len, true);
}

size_t string_tokenize_filter(const char *text, char delimiter, char *dst, size_t dst_len,
                              bool filter_empty)
{
	register const char *s = tokenized_param;

	if (text != NULL) {
		s = text;
	}

	// Skip leading delimiter characters.
	if (filter_empty) {
		while (*s == delimiter) { ++s; }
	}
	else {
		if (*s == delimiter) { ++s; }	
	}

	const char *start = s;

	char *d = dst;

	// Copy into the destination buffer until the string end or delimiter is met.
	while (*s && dst_len-- > 0) {

		if (*s == delimiter) {
			break;
		}

		*d++ = *s++;
	}

	// Null terminate the destination buffer.
	*d = 0;
	tokenized_param = s;

	// Return the number of characters in the token.
	return (s - start);
}

size_t string_tokenize_end(char delimiter, char *dst, size_t dst_len)
{
	register const char *s = tokenized_param;
	
	// Skip leading delimiter characters.
	while (*s == delimiter) { ++s; }

	const char *start = s;
	char *d = dst;

	// Copy into the destination buffer until the end of the string is reached.
	while (*s && dst_len-- > 0) {
		*d++ = *s++;
	}

	// Null terminate the destination buffer.
	*d = 0;
	tokenized_param = NULL;

	// Return the number of characters in the written buffer.
	return (s - start);
}

const char *string_last_token(const char *text, char delimiter)
{
	size_t l = strlen(text);

	while (l-- > 0) {
		if (text[l] == delimiter) {
			return &text[l + 1];
		}
	}

	return text;
}

size_t string_token_count(const char *text, char delimiter)
{
	if (string_is_null_or_empty(text)) {
		return 0;
	}

	register const char *s = text;
	size_t token_count = (*s != delimiter ? 1 : 0);

	for (; *s; s++) {

		// Skip all non-delimiter characters.
		if (*s != delimiter) {
			continue;
		}

		// Ignore multiple successive delimiters.
		while (*s == delimiter) { ++s; }

		// The string has ended.
		if (*s == 0) {
			break;
		}

		++token_count;
	}

	return token_count;
}

void string_parse_command(char *input, char **command, char **args)
{
	register char *s = input;

	*command = NULL;
	*args = NULL;

	// Strip leading whitespace.
	while (*s == ' ' || *s == '\t') {
		++s;
	}

	// Ignore comment lines.
	if (*s == '#' || *s == 0) {
		return;
	}

	// Get the command text and null-terminate it.
	*command = s;

	for (;;) {
		switch (*s) {
		case 0:
		case '\r':
		case '\n':
		case '\t':
		case ' ':
			goto parse_args;

		default:
			++s;
			break;
		}
	}

parse_args:

	if (*s == 0) {
		*args = s;
		return;
	}

	*s++ = 0;

	// Strip leading whitespace.
	while (*s == ' ' || *s == '\t') {
		++s;
	}

	// Get the arguments text and null-terminate it.
	*args = s;

	for (;;) {
		switch (*s) {
		case 0:
		case '\r':
		case '\n':
			*s = 0;
			return;

		default:
			++s;
			break;
		}
	}
}

void string_parse_device_id(const char *text, char delimiter, char *dst, size_t dst_len)
{
	register const char *s = text;
	*dst = 0;

	// Skip leading characters before the delimiter.
	while (*s != delimiter) { if (!*s) return; ++s; }
	while (*s == delimiter) { ++s; }

	char *d = dst;

	// Copy into the destination buffer until the string end or delimiter is met.
	while (*s && dst_len-- > 0) {
		if (*s == delimiter) {
			break;
		}

		*d++ = *s++;
	}

	// Null terminate the destination buffer.
	*d = 0;
}

char *string_strip(char **input)
{
	register char *s = *input;

	while (*s) {

		switch (*s) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				++s;
				continue;
		}

		break;
	}

	*input = s;
	return s;
}

char *string_strip_end(char **input)
{
	// Nothing to strip from an empty string.
	if (string_is_null_or_empty(*input)) {
		return *input;
	}

	// Get the end of the string.
	register char *s = *input + strlen(*input) - 1;

	// Skip characters until a non-white space character is found.
	while (s != *input) {

		switch (*s) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				*s-- = 0;
				continue;
		}

		break;
	}

	return *input;
}

bool string_is_numeric(const char *text)
{
	while (*text) {
		if (*text < '0' || *text > '9') {
			return false;
		}

		++text;
	}

	return true;
}

bool string_is_decimal(const char *text)
{
	while (*text) {
		if (*text < '0' || *text > '9' || *text != '.') {
			return false;
		}

		++text;
	}

	return true;
}

void string_get_file_extension(const char *text, char *dst, size_t dst_len)
{
	if (dst == NULL) {
		return;
	}

	*dst = 0;

	if (string_is_null_or_empty(text)) {
		return;
	}

	size_t len = strlen(text);
	const char *file = &text[len - 1];

	bool dot_was_found = false;

	// Find the dot which marks the beginning of the extension.
	while (len-- > 0) {

		if (text[len] == '.') {

			dot_was_found = true;
			break;
		}

		file = &text[len];
	}

	// Copy the extension into the destination buffer, assuming a dot was found in the filename.
	if (dot_was_found) {
		string_copy(dst, file, dst_len);
	}
}

void string_get_file_name_without_extension(const char *text, char *dst, size_t dst_len)
{
	if (dst == NULL) {
		return;
	}

	*dst = 0;

	if (string_is_null_or_empty(text)) {
		return;
	}

	size_t len = strlen(text);
	const char *file = &text[len - 1];

	// Find the first path separator from the end. This starts the filename.
	while (len-- > 0) {

		if (text[len] == '/') {
			break;
		}

		file = &text[len];
	}

	// Copy the filename into the destination buffer until a dot is found.
	while (*file && dst_len-- > 0) {
		if (*file == '.') {
			break;
		}

		*dst++ = *file++;
	}

	// Null terminate the destination buffer.
	*dst = 0;
}
