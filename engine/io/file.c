#include "file.h"
#include "core/string.h"
#include <stdio.h>

#ifndef _WIN32
	#include <dirent.h>
	#include <sys/sysinfo.h>

	#ifndef DT_DIR
	#define DT_DIR 0x4
	#endif
#endif

// --------------------------------------------------------------------------------

void file_for_each_in_directory(const char *directory, const char *extension, void (*method)(const char *file))
{
#ifdef _WIN32
	#error "file_for_each_in_directory is not implemented on Windows."
#else

	if (method == NULL) {
		return;
	}

	// Load and parse all config files from the given directory.
	DIR *dir = opendir(directory);

	if (dir == NULL) {
		return;
	}

	struct dirent *ent;

	while ((ent = readdir(dir)) != NULL)
	{
		char *dot = strrchr(ent->d_name, '.');

		if (ent->d_type != DT_DIR && // Make sure the entry is a regular file...
			(dot == NULL || string_equals(dot, extension))) // ...and the extension is the wanted one.
		{
			char path[260];
			snprintf(path, sizeof(path), "%s/%s", directory, ent->d_name);

			// Process the file.
			method(path);
		}
	}
#endif
}

void file_for_each_line(const char *path, void (*method)(char *line, size_t line_len))
{
	if (method == NULL) {
		return;
	}

	// Try to open the file.
	FILE *file = fopen(path, "r");

	if (file == NULL) {
		return;
	}

	char line[1024];
	register char c = 0;
	size_t i = 0;

	while (c != EOF) {

		// Read the file line by line.
		do {
			c = getc(file);

			if (c == EOF || c == '\n' || c == '\r') {
				break;
			}

			line[i++] = c;
		} while (c != '\0' && c != -1 && i < sizeof(line) - 1);

		// Null-terminate the line and handle it.
		if (i != 0) {
			line[i] = 0;
			method(line, i);

			i = 0;
		}
	}

	fclose(file);
}
