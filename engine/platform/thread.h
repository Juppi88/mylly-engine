#pragma once
#ifndef __THREAD_H
#define __THREAD_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

#ifdef _WIN32
	#include <Windows.h>

	typedef uint32_t (__stdcall *thread_t)(void *args);
	#define THREAD(x) static uint32_t __stdcall x(void *args)

	typedef CRITICAL_SECTION lock_t;
#else
	#include <pthread.h>

	typedef void *(*thread_t)(void *args);
	#define THREAD(x) static void *x(void *args)

	typedef pthread_mutex_t lock_t;
#endif

void thread_create(thread_t method, void *args);
void thread_sleep(uint32_t ms);

void thread_init_lock(lock_t *lock);
void thread_lock(lock_t *lock);
void thread_unlock(lock_t *lock);
void thread_destroy_lock(lock_t *lock);

END_DECLARATIONS;

#endif
