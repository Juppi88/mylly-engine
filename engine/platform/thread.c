#include "thread.h"

#ifdef _WIN32

#include <Windows.h>
#include <process.h>

void thread_create(thread_t method, void *args)
{
	uint32_t thread_addr;
	HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, method, args, 0, &thread_addr);

	if (thread != NULL) {
		CloseHandle(thread);
	}
}

void thread_sleep(uint32_t ms)
{
	Sleep(ms);
}

#else

#include <time.h>

void thread_create(thread_t method, void *args)
{
	pthread_t thread;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, method, args);
}

void thread_sleep(uint32_t ms)
{
	long seconds = (long)ms / 1000L;
	long millis = (long)ms - 1000L * seconds;

	struct timespec t;
	t.tv_sec = seconds;
	t.tv_nsec = 1000000L * millis;

	nanosleep(&t, NULL);
}

void thread_init_lock(lock_t *lock)
{
	pthread_mutex_init(lock, NULL);
}

void thread_lock(lock_t *lock)
{
	pthread_mutex_lock(lock);
}

void thread_unlock(lock_t *lock)
{
	pthread_mutex_unlock(lock);
}

void thread_destroy_lock(lock_t *lock)
{
	pthread_mutex_destroy(lock);
}

#endif
