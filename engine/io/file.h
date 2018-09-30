#pragma once
#ifndef __FILE_H
#define __FILE_H

#include "core/defines.h"

void file_for_each_in_directory(const char *directory, const char *extension, void (*method)(const char *file));
void file_for_each_line(const char *path, void (*method)(char *line, size_t line_len));

bool file_read_all_text(const char *path, char **buf, size_t *bytes_read);
bool file_read_all_data(const char *path, void **buf, size_t *bytes_read);

#endif
