#pragma once
#ifndef __LOG_H
#define __LOG_H

void log_message(const char *note, const char *format, ...);
void log_note(const char *note, const char *format, ...);
void log_error(const char *note, const char *format, ...);

#endif
