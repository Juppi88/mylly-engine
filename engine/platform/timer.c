#include "timer.h"

bool timer_has_expired(timeout_t *timer, uint64_t interval_ms)
{
	uint64_t ticks = timer_get_ticks();

	// The timer hasn't started yet, start it.
	if (*timer == 0) {
		*timer = ticks + interval_ms;
	}

	if (ticks < *timer) {
		return false;
	}

	// The timer has expired, reset it.
	*timer = 0;
	return true;
}

void timer_reset(timeout_t *timer, uint64_t interval_ms)
{
	uint64_t ticks = timer_get_ticks();
	*timer = ticks + interval_ms;
}

// --------------------------------------------------------------------------------

#ifdef _WIN32

#include <Windows.h>

uint64_t timer_get_ticks(void)
{
	return GetTickCount64();
}

#else

#include <time.h>

uint64_t timer_get_ticks(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	return (uint64_t)((now.tv_sec * 1000000000LL + now.tv_nsec) / 1000000LL);
}

#endif
