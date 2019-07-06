#include "platform.h"

// -------------------------------------------------------------------------------------------------

#ifndef _WIN32

#include <unistd.h>

void platform_set_working_directory(void)
{
	// Get the full path for the executable.
	char path[260];
	size_t read = readlink("/proc/self/exe", path, sizeof(path));

	// Null terminate the path and remove the name of the binary from it.
	path[read] = 0;

	for (size_t i = read; i > 0; i--) {
		if (path[i] == '/') {
			path[i] = 0;
			break;
		}
	}

	// Set working directory to the path of the executable.
	UNUSED_RETURN(chdir(path));
}

#else

#include <Windows.h>

void platform_set_working_directory(void)
{
	// Get the full path for the executable.
	HMODULE executable = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];

	DWORD read = GetModuleFileNameW(executable, path, MAX_PATH);

	// Null terminate the path and remove the name of the binary from it.
	path[read] = 0;

	for (DWORD i = read; i > 0; i--) {

		if (path[i] == '/') {
			path[i] = 0;
			break;
		}
	}

	// Set working directory to the path of the executable.
	SetCurrentDirectoryW(path);
}

#endif
