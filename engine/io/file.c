#include "file.h"
#include "core/string.h"
#include "core/memory.h"
#include <stdio.h>

#ifndef _WIN32
	#include <dirent.h>
	#include <sys/sysinfo.h>

	#ifndef DT_DIR
	#define DT_DIR 0x4
	#endif
#else
	#include <Windows.h>
#endif

// --------------------------------------------------------------------------------

static size_t file_get_size(FILE *file);

// --------------------------------------------------------------------------------

void file_for_each_in_directory(const char *directory,
								const char *extension, void (*method)(const char *file))
{
	if (method == NULL) {
		return;
	}

#ifdef _WIN32
	
	// Format a search path.
	char path[MAX_PATH];
	sprintf(path, "%s/*.%s", directory, extension);

	// Perform a find in the specified path.
	WIN32_FIND_DATA find_data;
	HANDLE find = FindFirstFileA(path, &find_data);

	if (find == INVALID_HANDLE_VALUE) {
		return;
	}

	// Iterate find results.
	do {
		// Find will return the current and parent directories as the first two results.
		if (string_equals(find_data.cFileName, ".") ||
			string_equals(find_data.cFileName, "..")) {

			continue;
		}

		// Skip directories.
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// Process the file.
		snprintf(path, sizeof(path), "%s/%s", directory, find_data.cFileName);
		method(path);
	}
	while (FindNextFile(find, &find_data)); 

	// Close the find handle.
	FindClose(find);

#else

	// Find and iterate all files in the given directory.
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

bool file_for_each_line(const char *path,
						void (*method)(char *line, size_t line_len, void *context),
						void *context,
						bool keep_new_line)
{
	if (method == NULL) {
		return false;
	}

	// Try to open the file.
	FILE *file = fopen(path, "r");

	if (file == NULL) {
		return false;
	}

	char line[1024];
	register char c = 0;
	size_t i = 0;

	while (c != EOF) {

		// Read the file line by line.
		do {
			c = getc(file);

			// Keep line endings for each line when keep_new_line is set.
			if (keep_new_line && (c == '\n' || c == '\r')) {
				line[i++] = '\n';
			}

			// Break on line change.
			if (c == EOF || c == '\n' || c == '\r') {
				break;
			}

			line[i++] = c;
		} while (c != '\0' && c != -1 && i < sizeof(line) - 1);

		// Null-terminate the line and handle it.
		if (i != 0) {
			line[i] = 0;
			method(line, i, context);

			i = 0;
		}
	}

	fclose(file);

	return true;
}

bool file_read_all_text(const char *path, char **buf, size_t *bytes_read)
{
	*buf = NULL;
	*bytes_read = 0;

	// Open the file for reading.
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		return false;
	}

	// Get the size of the file.
	size_t size = file_get_size(f);

	// Allocate a big enough buffer for the contents of the file.
	char *buffer = mem_alloc_fast(size + 1);

	// Read the file into the buffer and close the handle.
	fread(buffer, size, 1, f);
	buffer[size] = 0;

	fclose(f);

	*buf = buffer;
	*bytes_read = size;

	return true;
}

bool file_read_all_data(const char *path, void **buf, size_t *bytes_read)
{
	*buf = NULL;
	*bytes_read = 0;

	// Open the file for reading.
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		return false;
	}

	// Get the size of the file.
	size_t size = file_get_size(f);

	// Allocate a big enough buffer for the contents of the file.
	char *buffer = mem_alloc_fast(size);

	// Read the file into the buffer and close the handle.
	fread(buffer, size, 1, f);
	fclose(f);

	*buf = buffer;
	*bytes_read = size;

	return true;
}

static size_t file_get_size(FILE *file)
{
	fseek(file, 0, SEEK_END);
	size_t size = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);

	return size;
}
